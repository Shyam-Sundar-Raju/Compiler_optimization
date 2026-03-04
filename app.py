import os
import shlex
import subprocess
import tempfile
from pathlib import Path

import streamlit as st

PROJECT_DIR = Path(__file__).resolve().parent
OPTIMIZER_BIN = PROJECT_DIR / "main"
SOURCE_MAIN = PROJECT_DIR / "main.c"

OPTIMIZATIONS = {
    1: "algebraic_simplification",
    2: "constant_propagation",
    3: "copy_propagation",
    4: "constant_folding",
    5: "common_subexpression_elimination",
    6: "strength_reduction",
    7: "function_inlining",
    8: "function_cloning",
    9: "loop_invariant_code_motion",
    10: "induction_variable_elimination",
    11: "loop_fusion",
    12: "loop_peeling",
    13: "loop_unrolling",
    14: "dead_code_elimination",
    15: "unreachable_code_elimination",
}


def ensure_optimizer_binary() -> tuple[bool, str]:
    if OPTIMIZER_BIN.exists() and os.access(OPTIMIZER_BIN, os.X_OK):
        return True, ""

    if not SOURCE_MAIN.exists():
        return False, f"Could not find source file: {SOURCE_MAIN}"

    compile_cmd = ["gcc", str(SOURCE_MAIN), "-o", str(OPTIMIZER_BIN)]
    proc = subprocess.run(compile_cmd, cwd=PROJECT_DIR, capture_output=True, text=True)

    if proc.returncode != 0:
        details = (proc.stderr or proc.stdout or "Unknown compilation error").strip()
        return False, f"Failed to build optimizer using: {' '.join(shlex.quote(x) for x in compile_cmd)}\n\n{details}"

    return True, ""


def run_optimizer(c_file: Path, optimization_id: int | None) -> tuple[int, str, str]:
    cmd = [str(OPTIMIZER_BIN), str(c_file)]
    if optimization_id is not None:
        cmd.append(str(optimization_id))

    proc = subprocess.run(cmd, cwd=PROJECT_DIR, capture_output=True, text=True)
    return proc.returncode, proc.stdout, proc.stderr


def split_before_after(text: str) -> tuple[str, str]:
    original_marker = "Original GIMPLE:"
    optimized_marker = "Optimized GIMPLE:"

    original = ""
    optimized = ""

    if original_marker in text and optimized_marker in text:
        after_original = text.split(original_marker, maxsplit=1)[1]
        original, optimized = after_original.split(optimized_marker, maxsplit=1)
        return original.strip(), optimized.strip()

    return text.strip(), "Could not parse optimized section from output."


st.set_page_config(page_title="C Optimizer GUI", layout="wide")
st.title("C Optimization Viewer (3-Address/GIMPLE)")
st.write("Upload a `.c` file, choose one optimization (or all), and compare before/after output.")

uploaded_file = st.file_uploader("Upload C source file", type=["c"])
selected = st.selectbox(
    "Choose optimization",
    options=["All optimizations"] + [f"{idx}. {name}" for idx, name in OPTIMIZATIONS.items()],
)

run_clicked = st.button("Run Optimization", type="primary")

if run_clicked:
    if uploaded_file is None:
        st.error("Please upload a `.c` file first.")
        st.stop()

    ok, err = ensure_optimizer_binary()
    if not ok:
        st.error(err)
        st.stop()

    with tempfile.NamedTemporaryFile(delete=False, suffix=".c") as tmp:
        tmp.write(uploaded_file.getvalue())
        tmp_path = Path(tmp.name)

    try:
        optimization_id = None
        if selected != "All optimizations":
            optimization_id = int(selected.split(".", maxsplit=1)[0])

        with st.spinner("Running optimizer..."):
            code, stdout, stderr = run_optimizer(tmp_path, optimization_id)

        if code != 0:
            st.error("Optimizer execution failed.")
            if stdout.strip():
                st.code(stdout, language="text")
            if stderr.strip():
                st.code(stderr, language="text")
            st.stop()

        before, after = split_before_after(stdout)

        left, right = st.columns(2)
        with left:
            st.subheader("Before Optimization")
            st.code(before if before else "No pre-optimization output found.", language="text")
        with right:
            st.subheader("After Optimization")
            st.code(after if after else "No optimized output found.", language="text")

        if stderr.strip():
            with st.expander("Debug logs (stderr)"):
                st.code(stderr, language="text")

    finally:
        try:
            tmp_path.unlink(missing_ok=True)
        except Exception:
            pass
