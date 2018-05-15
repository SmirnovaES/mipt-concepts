class LINQ:

    def __init__(self, generator):
        self.generator = generator

    def select(self, func):
        def new_generator():
            for elem in self.generator():
                yield func(elem)
        return LINQ(new_generator)

    def flatten(self):
        def new_generator():
            for elem in self.generator():
                for subelem in elem:
                    yield subelem
        return LINQ(new_generator)

    def where(self, func):
        def new_generator():
            for elem in self.generator():
                if func(elem):
                    yield elem
        return LINQ(new_generator)

    def take(self, n):
        def new_generator():
            prev_generator = self.generator()
            for i in range(n):
                next_elem = next(prev_generator)
                print("%d is generated" % next_elem)
                yield next_elem
        return LINQ(new_generator)

    def to_list(self):
        return [elem for elem in self.generator()]

    def group_by(self, func):
        key_elems = {}
        for elem in self.generator():
            if func(elem) not in key_elems:
                key_elems[func(elem)] = []
            key_elems[func(elem)].append(elem)

        def new_generator():
            for key, elems in key_elems.items():
                yield key, elems

        return LINQ(new_generator)

    def order_by(self, func):
        sorted_elems = sorted([elem for elem in self.generator()], key=func)

        def new_generator():
            for elem in sorted_elems:
                yield elem
        return LINQ(new_generator)

