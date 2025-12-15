import haruka.syntax_tree as st

types = {"u64": 8}
args = ["rdi", "rsi"]
register = {"a": "rax", "D": "rdi"}


def calc_offset(args: dict[str, str], elements: st.Elements) -> int:
    offset = 0
    for element in elements:
        if type(element) is st.Variable:
            assert type(element) is st.Variable
            offset -= types[element.variable_type]
    return offset


def function_to_asm(function: st.Function) -> str:
    result = f"    .globl {function.name}\n{function.name}:\n"  # start
    result += "    pushq   %rbp\n    movq    %rsp, %rbp\n"
    offset = calc_offset(function.args, function.body)
    offsets: dict[str, int] = {}
    arg_offset = offset
    for key, value in function.args.items():
        arg_offset -= types[value]
        offsets[key] = arg_offset
    # put args on stack
    for i, key in enumerate(function.args.keys()):
        result += f"    movq    %{args[i]}, {offsets[key]}(%rbp)\n"
    for element in function.body:
        match type(element):
            case st.Variable:
                assert type(element) is st.Variable
                offsets[element.name] = offset
                offset += types[element.variable_type]
            case st.Assembler:
                assert type(element) is st.Assembler
                for key, value in element.input_operands.items():
                    result += f"    movq    {offsets[value]}(%rbp), %{register[key]}\n"
                result += f"    {element.template}\n"
                for key, value in element.output_operands.items():
                    result += (
                        f"    movq    %{register[key[1:]]}, {offsets[value]}(%rbp)\n"
                    )
            case st.Return:
                assert type(element) is st.Return
                result += f"    movq    {offsets[element.value]}(%rbp), %rax\n"
            case st.FunctionCall:
                assert type(element) is st.FunctionCall
                if element.args:
                    for i, arg in enumerate(element.args):
                        result += f"    movq    ${arg}, %{args[i]}\n"
                result += f"    call    {element.name}\n"
            case _:
                raise Exception(f"{type(element)} is not implemented")
    print(offsets)
    result += "    popq    %rbp\n    ret\n"
    return result


def element_to_asm(element: st.Element) -> str:
    match type(element):
        case st.Function:
            assert type(element) is st.Function
            return function_to_asm(element)
        case _:
            raise Exception(f"{type(element)} is not implemented")


def compile_to_asm(syntax_tree: st.Elements):
    asm_code = ""
    for element in syntax_tree:
        asm_code += element_to_asm(element)
    return asm_code
    return asm_code
