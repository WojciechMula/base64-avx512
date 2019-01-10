%.base64: %.jpg
	base64 $< > $@

%.base64: %.png
	base64 $< > $@

JPG=$(wildcard *.jpg)
PNG=$(wildcard *.png)
ALL=$(JPG:jpg=base64) $(PNG:png=base64)

all: $(ALL)
