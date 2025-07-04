import subprocess
import filecmp
import pytest
from pathlib import Path

TEST_DIR = Path("tests")
BINARY_PATH = Path(__file__).parent / "test2"

# Collect all .in files
def get_test_files():
    return sorted(TEST_DIR.glob("*.in"))

@pytest.mark.parametrize("input_file", get_test_files())
def test_input_output(input_file):
    expected_output_file = input_file.with_suffix(".out")
    assert expected_output_file.exists(), f"Expected file missing for {input_file.name}"

    with open(input_file, "r") as f_in:
        result = subprocess.run(
            [str(BINARY_PATH)],
            stdin=f_in,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True
        )

    output_path = input_file.with_suffix(".actual")
    with open(output_path, "w") as f_out:
        f_out.write(result.stdout)

    with open(expected_output_file, "r") as f_exp, open(output_path, "r") as f_act:
        expected_content = f_exp.read().strip()
        actual_content = f_act.read().strip()

    assert expected_content == actual_content, (
        f"\n\033[91mFAIL:\033[0m {input_file.name}\n"
        f"---- Expected ----\n{expected_content}\n"
        f"---- Actual   ----\n{actual_content}\n"
    )
