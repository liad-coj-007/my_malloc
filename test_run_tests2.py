import subprocess
import pytest
from pathlib import Path
import difflib

TEST_DIR = Path("my_tests/tests2")
BINARY_PATH = Path(__file__).parent / "test2"

def get_test_files():
    return sorted(TEST_DIR.glob("*.in"))

def color_diff_line(line: str, expected_ln: int, actual_ln: int) -> str:
    prefix = line[:2]
    content = line[2:]
    ln_width = 4  # width for line numbers
    if prefix == '- ':
        # Removed line (expected)
        return f"\033[91m- {expected_ln:>{ln_width}} | {'':>{ln_width}} | {content}\033[0m"
    elif prefix == '+ ':
        # Added line (actual)
        return f"\033[92m  {'':>{ln_width}} | {actual_ln:>{ln_width}} | {content}\033[0m"
    else:
        return line

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
        expected_lines = f_exp.read().splitlines()
        actual_lines = f_act.read().splitlines()

    if expected_lines != actual_lines:
        diff_lines = list(difflib.ndiff(expected_lines, actual_lines))

        expected_line_no = 1
        actual_line_no = 1
        formatted_diff = []

        for line in diff_lines:
            if line.startswith('  '):
                expected_line_no += 1
                actual_line_no += 1
            elif line.startswith('- '):
                formatted_diff.append(color_diff_line(line, expected_line_no, 0))
                expected_line_no += 1
            elif line.startswith('+ '):
                formatted_diff.append(color_diff_line(line, 0, actual_line_no))
                actual_line_no += 1
            elif line.startswith('? '):
                # skip hint lines
                continue

        header = f"{'':>2} {'Exp':>4} | {'Act':>4} | Content"
        final_diff = "\n".join(formatted_diff)

        pytest.fail(
            f"\n\033[91mFAIL:\033[0m {input_file.name}\n"
            f"{final_diff}\n"
        )
    else:
        print(f"\033[92mPASS:\033[0m {input_file.name}")
