import time

def fib(n):
    if n < 2:
        return n
    return fib(n - 2) + fib(n - 1)

start = time.time()
print("Start")

for i in range(32):
    print(fib(i))

end = time.time()
print("Time", end - start)

