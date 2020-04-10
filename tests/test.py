from itertools import product
from sys import argv
import subprocess
import os
import pathlib

BOLD = '\033[1m'
OKGREEN = '\033[92m'
FAIL    = '\033[91m'
ENDC    = '\033[0m'

OPTIONS = [
    ["", "-a"], # , "--all"
    ["", "-b"], # , "--bytes"
    ["", "-B 1024", "-B 2048", "-B 512", "-B 1", "--block-size=1"], # "--block-size=1024", "--block-size=2048", "--block-size=512",
    ["-l"],
    ["", "-L"], # , "--dereference"
    ["", "-S"], # , "--separate-dirs"
    ["", "--max-depth=0", "--max-depth=1", "--max-depth=2"], # , "--max-depth=3", "--max-depth=12974"
    ["", "..", "src", "tests"], #  ".",
]

PERMUTATIONS = list(product(*OPTIONS))

PROJECT_DIR = pathlib.Path(__file__).absolute().parents[1]
SOURCE_DIR     = PROJECT_DIR / "src"

def test(options : iter):

    commandsList = list(filter(lambda i : i != "", options))
    print(f"Testing src/simpledu {' '.join(commandsList)}")

    subprocess.check_output(f"(src/simpledu {' '.join(commandsList)} || echo $?) | sort -k2 > /tmp/testeT_simpledu.txt", shell=True)
    subprocess.check_output(f"(du           {' '.join(commandsList)} || echo $?) | sort -k2 > /tmp/testeT_du.txt", shell=True)
    output = subprocess.check_output("diff -q /tmp/testeT_simpledu.txt /tmp/testeT_du.txt > /dev/null 2>&1 && echo OK || echo FAILED", shell=True).decode("utf-8").strip()

    if (output == "OK"):
        print(f"{BOLD}{OKGREEN}    Success!{ENDC}")
        return True
    else:
        print(f"{BOLD}{FAIL}    Failure{ENDC}")
        print("==============================DU=OUTPUT==============================")
        print(subprocess.check_output(f"du           {' '.join(commandsList)}", shell=True).decode("utf-8"))
        print("===========================SIMPLEDU=OUTPUT===========================")
        print(subprocess.check_output(f"src/simpledu {' '.join(commandsList)}", shell=True).decode("utf-8"))
        print("=====================================================================")
        return False
        
def run_exaustive_test():
    print("Setting working directory to project root...")
    os.chdir(PROJECT_DIR)

    fails  = []
    passes = []

    for i, perm in enumerate(PERMUTATIONS):
        print()
        print(f"[{i}]")
        try:
            if test(perm):
                passes.append(i)
            else:
                fails.append(i)
        except KeyboardInterrupt:
            print("\nStopping the tests")
            break

    print()
    print()
    print()
    print("Failed tests:")
    print(" ".join(map(str, fails)))

    print(f"Total tests : {len(fails) + len(passes)}")
    print(f"Fails       : {len(fails)}")
    print(f"Passes      : {len(passes)}")
    print()
    percentage_passed = (len(passes) / (len(fails) + len(passes))) * 100
    print(f"{percentage_passed}% of tests passed!")

def run_single_test(i : int):
    perm = PERMUTATIONS[i]
    print()
    print(f"[{i}]")
    test(perm)

def make_clean_make():
    print("Setting working directory to source root...")
    os.chdir(SOURCE_DIR)

    print("Compiling...")
    subprocess.run("make clean; make", shell = True)
    print("simpledu has been compiled!")

if __name__ == "__main__":
    make_clean_make()

    os.environ["LOG_FILENAME"] = "/tmp/simpleduLog.txt"

    if len(argv) == 1:
        run_exaustive_test()
    else:
        print("Setting working directory to project root...")
        os.chdir(PROJECT_DIR)
        
        for i in map(int, argv[1:]):
            run_single_test(i)