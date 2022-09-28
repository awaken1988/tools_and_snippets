
def decorator_asterisk(func):
    def helper(argument):
        return "*{}*".format(func(argument))
    return helper

def decorator_spaces(func):
    def helper(argument):
        return " {} ".format(func(argument))
    return helper


@decorator_asterisk
@decorator_spaces
def show_string(argument):
    return argument

print(show_string("test"))
