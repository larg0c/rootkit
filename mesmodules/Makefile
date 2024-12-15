obj-m += rv.o

PWD := $(CURDIR)

build:
	make -C ../linux-6.10 M=$(PWD) modules

clean:
	make -C ../linux-6.10 M=$(PWD) clean
