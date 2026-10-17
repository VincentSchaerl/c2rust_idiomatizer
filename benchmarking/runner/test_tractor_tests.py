"""Fast isolated regression tests for TRACTOR JUnit parsing and staging."""
import contextlib
import io
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
from tractor_tests import VectorResults, _read_junit, run_tractor_vectors, summarize_tests

XML = '''<?xml version="1.0"?>
<testsuites>
  <testsuite name="Public-Tests/B02_organic/arr_del_lib">
    <testcase name="config" />
    <testcase name="build" />
    <testcase name="build-runners" />
    <testcase name="1.json" vector_code="0" />
    <testcase name="2.json" vector_code="0"><skipped message="UB"/></testcase>
    <testcase name="3.json" vector_code="2"><failure message="wrong output"/></testcase>
  </testsuite>
</testsuites>
'''

class TestTractor(unittest.TestCase):
    def test_junit(self):
        with tempfile.TemporaryDirectory() as directory:
            xml=Path(directory)/'junit.xml'
            xml.write_text(XML)
            result=_read_junit(xml, 'B02_organic/arr_del_lib')
            self.assertEqual((result.passed,result.failed,result.skipped,result.status), (1,1,1,'failed'))
            self.assertIn('wrong output', ' '.join(result.details))
            with self.assertRaisesRegex(ValueError,'Expected exactly one'):
                _read_junit(xml, 'B02_organic/nope')

    def test_no_vectors(self):
        with tempfile.TemporaryDirectory() as directory:
            xml=Path(directory)/'junit.xml'
            xml.write_text('<testsuites><testsuite name="Public-Tests/B01_synthetic/x"><testcase name="build" /></testsuite></testsuites>')
            with self.assertRaisesRegex(ValueError,'no test vectors'):
                _read_junit(xml,'B01_synthetic/x')

    def test_staging_cleanup_and_commands(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            case_dir=root/'Public-Tests'/'B02_organic'/'arr_del_lib'
            case_dir.mkdir(parents=True)
            (root/'tools'/'test_runner').mkdir(parents=True)
            project=root/'myproject'
            project.mkdir()
            (project/'Cargo.toml').write_text('[package]\nname="transpile"\n')
            (project/'src').mkdir()
            (project/'src'/'lib.rs').write_text('fn whatever(){}')
            (project/'target').mkdir()
            (project/'target'/'large.bin').write_bytes(b'large')
            case=SimpleNamespace(case_dir=case_dir,case_id='B02_organic/arr_del_lib',kind='library')
            observed=[]
            def fake_run(cmd, **kwargs):
                staged=case_dir/'translated_rust'
                self.assertTrue(staged.is_dir())
                self.assertFalse((staged/'target').exists())
                self.assertTrue((staged/'src'/'lib.rs').exists())
                manifest = (staged/'Cargo.toml').read_text()
                self.assertIn('[lib]', manifest)
                self.assertIn('name = "arr_del_lib"', manifest)
                self.assertIn('crate-type = ["cdylib"]', manifest)
                self.assertIn('--rust', cmd)
                self.assertEqual(kwargs['cwd'],root)
                index=cmd.index('--junit-xml')
                Path(cmd[index+1]).write_text(XML)
                observed.append(cmd)
                return subprocess.CompletedProcess(cmd,1)
            with patch('tractor_tests._test_prerequisites',return_value=None), patch('tractor_tests.subprocess.run',side_effect=fake_run):
                r=run_tractor_vectors(case,'refactored',project,root/'logs')
            self.assertEqual(r.status,'failed')
            self.assertFalse((case_dir/'translated_rust').exists())
            self.assertNotIn('[lib]', (project/'Cargo.toml').read_text())
            self.assertEqual(len(observed),1)


    def test_existing_lib_settings_preserve_crate_path(self):
        from tractor_tests import _prepare_tractor_library
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'lib.rs').write_text('pub extern "C" fn example() {}')
            cargo = root / 'Cargo.toml'
            cargo.write_text('[package]\nname = "transpile"\nversion="0.0.0"\n\n[lib]\nname = "transpile"\npath = "lib.rs"\ncrate-type = ["rlib"]\n\n[dependencies]\n')
            _prepare_tractor_library(cargo, 'arr_del_lib')
            import tomllib
            parsed = tomllib.loads(cargo.read_text())
            self.assertEqual(parsed['lib'], {'name': 'arr_del_lib', 'path':'lib.rs', 'crate-type':['cdylib']})
            self.assertEqual(parsed['package']['name'], 'transpile')

    def test_explicit_library_name_in_cando_runner(self):
        from tractor_tests import _tractor_library_name
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            src = root / 'runner' / 'src'
            src.mkdir(parents=True)
            (src / 'main.rs').write_text('harness! {\n  library: "hello",\n  symbol: "helloworld",\n}\n')
            case = SimpleNamespace(case_dir=root, case_id='B01_synthetic/001_helloworld_lib')
            self.assertEqual(_tractor_library_name(case), 'hello')

    def test_explicit_binary_target_renamed_without_changing_path(self):
        from tractor_tests import _prepare_tractor_binary
        import tomllib
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            cargo = root / 'Cargo.toml'
            cargo.write_text('[package]\nname="transpile"\nversion="0.0.0"\n'
                             '[[bin]]\nname="main"\npath="src/main.rs"\n'
                             '[dependencies]\n')
            (root / 'src').mkdir()
            (root / 'src' / 'main.rs').write_text('fn main() {}')
            _prepare_tractor_binary(cargo, 'driver', 'main')
            parsed=tomllib.loads(cargo.read_text())
            self.assertEqual(parsed['bin'], [{'name':'driver','path':'src/main.rs'}])
            self.assertEqual(parsed['package']['name'], 'transpile')

    def test_implicit_binary_target_added(self):
        from tractor_tests import _prepare_tractor_binary
        import tomllib
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            cargo=root/'Cargo.toml'
            cargo.write_text('[package]\nname="transpile"\nversion="0.0.0"\n')
            (root/'src').mkdir()
            (root/'src'/'main.rs').write_text('fn main() {}')
            _prepare_tractor_binary(cargo,'driver','main')
            self.assertEqual(tomllib.loads(cargo.read_text())['bin'],
                             [{'name':'driver','path':'src/main.rs'}])

    def test_binary_target_multi_chooses_c_main(self):
        from tractor_tests import _prepare_tractor_binary
        import tomllib
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory); cargo=root/'Cargo.toml'
            cargo.write_text('[package]\nname="transpile"\nversion="0.0.0"\n'
                             '[[bin]]\nname="main"\npath="main.rs"\n'
                             '[[bin]]\nname="helper"\npath="helper.rs"\n')
            _prepare_tractor_binary(cargo,'driver','main')
            self.assertEqual(tomllib.loads(cargo.read_text())['bin'], [
                {'name':'driver','path':'main.rs'}, {'name':'helper','path':'helper.rs'}])

    def test_binary_staging_changes_only_copied_manifest(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            case_dir=root/'Public-Tests'/'B01_synthetic'/'001_helloworld'
            case_dir.mkdir(parents=True)
            (root/'tools'/'test_runner').mkdir(parents=True)
            project=root/'project'
            project.mkdir()
            (project/'src').mkdir()
            (project/'src'/'main.rs').write_text('fn main() {}')
            cargo=project/'Cargo.toml'
            original='[package]\nname="transpile"\nversion="0.0.0"\n'
            cargo.write_text(original)
            def fake_run(cmd, **kwargs):
                staged=case_dir/'translated_rust'
                self.assertIn('name = "driver"', (staged/'Cargo.toml').read_text())
                self.assertIn('--rust',cmd)
                self.assertFalse((staged/'target').exists())
                i=cmd.index('--junit-xml')
                Path(cmd[i+1]).write_text("""<testsuites><testsuite name=\"Public-Tests/B01_synthetic/001_helloworld\"><testcase name=\"test1.json\" /></testsuite></testsuites>""")
                return subprocess.CompletedProcess(cmd,0)
            case=SimpleNamespace(case_dir=case_dir,case_id='B01_synthetic/001_helloworld',kind='binary')
            with patch('tractor_tests._test_prerequisites',return_value=None), patch('tractor_tests.subprocess.run',side_effect=fake_run):
                result=run_tractor_vectors(case,'transpiled',project,root/'logs',binary_name='driver',main_module='main')
            self.assertEqual((result.passed,result.status),(1,'complete'))
            self.assertFalse((case_dir/'translated_rust').exists())
            self.assertEqual(cargo.read_text(), original)

    def test_missing_dependencies(self):
        case=SimpleNamespace(case_dir=Path('/does/not/matter'),case_id='B01/x',kind='binary')
        with tempfile.TemporaryDirectory() as directory, patch('tractor_tests._test_prerequisites',return_value='Missing nix'):
            r=run_tractor_vectors(case,'c',None,Path(directory))
        self.assertEqual(r.status,'not run')
        self.assertIn('nix',r.reason)

    def test_summarize(self):
        rows=[{'name':'tractor/B02_organic/arr_del_lib','kind':'library',
               'vector_tests':{'c':VectorResults(2,0,1,'complete'),
                 'transpiled':VectorResults(1,1,1,'failed'),
                 'refactored':VectorResults(status='not run',reason='compile failed')}}]
        s=summarize_tests(rows)
        self.assertIn('Original C',s)
        self.assertIn('Not run (Refactored (true))',s)
        self.assertIn('=== TRACTOR test vectors',s)
        self.assertNotIn('Not run (Transpiled)',s)

if __name__ == '__main__': unittest.main()
