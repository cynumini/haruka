import subprocess
import haruka.syntax_tree as st
from pathlib import Path

types_c = """#include <stdio.h>

int main() {
  printf("i%lu,signed char\\n", sizeof(signed char) * 8);
  printf("u%lu,unsigned char\\n", sizeof(unsigned char) * 8);
  printf("i%lu,short int\\n", sizeof(short int) * 8);
  printf("u%lu,unsigned short int\\n", sizeof(unsigned short int) * 8);
  printf("i%lu,int\\n", sizeof(int) * 8);
  printf("u%lu,unsigned int\\n", sizeof(unsigned int) * 8);
  printf("i%lu,long int\\n", sizeof(long int) * 8);
  printf("u%lu,unsigned long int\\n", sizeof(unsigned long int) * 8);
  printf("i%lu,long long int\\n", sizeof(long long int) * 8);
  printf("u%lu,unsigned long long int\\n", sizeof(unsigned long long int) * 8);
  printf("f%lu,float\\n", sizeof(float) * 8);
  printf("f%lu,double\\n", sizeof(double) * 8);
  printf("f%lu,long double\\n", sizeof(long double) * 8);
  return 0;
}"""


types = {"char": "char", "void": "void"}


def get_types():
    types_c_path = Path("./types.c")
    types_path = Path("./types")
    _ = types_c_path.write_text(types_c)
    _ = subprocess.run(["gcc", str(types_c_path), "-o", str(types_path)])
    result = subprocess.check_output(["./types"], text=True)
    types_c_path.unlink()
    types_path.unlink()

    for line in result.split("\n"):
        if not line:
            continue
        key, value = line.split(",")
        if key not in types:
            types[key] = value


def function_to_c(function: st.Function) -> str:
    result = f"{types[function.return_type]} {function.name}("
    for key, value in function.args.items():
        result += f"{types[value]} {key},"
    if result[-1] == ",":
        result = result[:-1]
    result += "){"
    for element in function.body:
        result += element_to_c(element)
    return result + "}"


def variable_to_c(variable: st.Variable) -> str:
    return f"{types[variable.variable_type]} {variable.name};"


def assembler_to_c(assembler: st.Assembler) -> str:
    result = "asm"
    if assembler.volatile:
        result += " volatile("
    else:
        result += "("
    result += f"{assembler.template}:"
    for key, value in assembler.output_operands.items():
        result += f"{key}({value}),"
    if result[-1] == ",":
        result = result[:-1]
    result += ":"
    for key, value in assembler.input_operands.items():
        result += f"{key}({value}),"
    if result[-1] == ",":
        result = result[:-1]
    result += ":"
    for clobber in assembler.clobbers:
        result += f"{clobber},"
    if result[-1] == ",":
        result = result[:-1]
    result += ");"
    return result


def return_to_c(return_element: st.Return) -> str:
    return f"return {return_element.value};"


def element_to_c(element: st.Element) -> str:
    match type(element):
        case st.Function:
            assert type(element) is st.Function
            return function_to_c(element)
        case st.Variable:
            assert type(element) is st.Variable
            return variable_to_c(element)
        case st.Assembler:
            assert type(element) is st.Assembler
            return assembler_to_c(element)
        case st.Return:
            assert type(element) is st.Return
            return return_to_c(element)
        case st.FunctionCall:
            assert type(element) is st.FunctionCall
            return function_call_to_c(element)
        case _:
            raise Exception(f"{type(element)} is not implemented")


def function_call_to_c(function_call: st.FunctionCall) -> str:
    result = f"{function_call.name}("
    for arg in function_call.args:
        result += f"{arg},"
    if result[-1] == ",":
        result = result[:-1]
    return result + ");"


def compile_to_c(syntax_tree: st.Elements):
    get_types()
    c_code = ""
    for element in syntax_tree:
        c_code += element_to_c(element)
    return c_code
