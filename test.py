class A:
    def __init__(self):
        self._a = 5

    def __abc(self):
        pass

print(vars(locals()['A']))

_A__abc