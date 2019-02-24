# Builds for 64-bit by default.
# To build for 32-bit replace "x86_64" with "i386".
# -Wno-format is enabled to suppress warnings from
# printf since some types have changed on 64-bit.
# Refer to NIDAQmxBase.h for changes.

nilibs=-framework nidaqmxbase -framework nidaqmxbaselv
flags= -O2 -arch x86_64 -F/Library/Frameworks -Wno-format
cc=gcc

ai_examples = acquireNScans acquireNScans-ExtClk acquireNScans-AnlgStart \
              acquireNScans-DigRef contAcquire-ExtClk-DigStart \
              contAcquireNChan acquire1Scan loadAITask \
              acquireNScansThermocouple
      
all : $(ai_examples)

% : %.c
	$(cc) $(flags) $< $(nilibs) -o $@

clean :
	rm -f $(ai_examples)
