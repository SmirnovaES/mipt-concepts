from linq import LINQ

#task 1
def fib():
    a, b = 0, 1
    while True:
        a, b = b, a + b
        yield a


print(LINQ(fib).where(lambda x: x % 3 == 0).select(lambda x: x if x % 2 != 0 else x ** 2).take(5).to_list())

#task 2
with open("in.txt", "r") as input:
    def read_generator():
        for string in input:
            words = [word for word in string.replace('\n', ' ').split(' ') if word]
            for word in words:
                yield word

    print(LINQ(read_generator).group_by(lambda x: x).select(lambda x: (len(x[1]), x[0])).order_by(lambda x: x[0]).to_list())