import subprocess
import pytest
from pathlib import Path
import difflib
import re

TEST_DIR = Path("tests/tests3")
BINARY_PATH = Path(__file__).parent / "test3"

FLAGS = {
    "ME": 32,
    "MH": 32 * ((2 ** 17) - 32),
    "MV": 2 ** 17
}

def get_test_files():
    return sorted(TEST_DIR.glob("*.in"))

def safe_eval(expr: str) -> str:
    # Replace constants
    for flag, val in FLAGS.items():
        expr = re.sub(r'\b' + re.escape(flag) + r'\b', str(val), expr)

    # Validate allowed characters and operators
    if not re.fullmatch(r'[\d\s+\-*/().**]+', expr):
        raise ValueError(f"Unsafe characters in expression: {expr}")

    try:
        result = eval(expr, {"__builtins__": None}, {})
    except Exception as e:
        raise ValueError(f"Error evaluating expression '{expr}': {e}")

    if isinstance(result, float) and result.is_integer():
        result = int(result)

    return str(result)

def eval_flag_expression(line: str) -> str:
    if ':' not in line:
        return line

    lhs, rhs = line.split(':', 1)
    rhs = rhs.strip()

    if not rhs:
        return line

    try:
        val = safe_eval(rhs)
    except ValueError:
        return line

    return f"{lhs}: {val}"

def color_diff_line(line: str, expected_ln: int, actual_ln: int) -> str:
    prefix = line[:2]
    content = line[2:]
    ln_width = 4
    if prefix == '- ':
        return f"\033[91m- {expected_ln:>{ln_width}} | {'':>{ln_width}} | {content}\033[0m"
    elif prefix == '+ ':
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
        expected_lines = [eval_flag_expression(line) for line in f_exp.read().splitlines()]
        actual_lines = f_act.read().splitlines()

    # Apply IG line filtering (skip both expected and actual line)
    filtered_expected = []
    filtered_actual = []
    for exp_line, act_line in zip(expected_lines, actual_lines):
        if exp_line.strip().startswith("IG"):
            continue
        filtered_expected.append(exp_line)
        filtered_actual.append(act_line)

    if filtered_expected != filtered_actual:
        diff_lines = list(difflib.ndiff(filtered_expected, filtered_actual))

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
                continue

        final_diff = "\n" + "\n".join(formatted_diff)

        pytest.fail(
            f"\n\033[91mFAIL:\033[0m {input_file.name}\n"
            f"{final_diff}\n"
        )
    else:
        print(f"\033[92mPASS:\033[0m {input_file.name}")
