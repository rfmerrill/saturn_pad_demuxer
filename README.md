# saturn_pad_demuxer
Demuxer for standard saturn pad, 12x active low outputs.

This is a program for ATTiny2313 that demuxes the Saturn 6-button pad's multiplexed interface and gives you 12 active low outputs that would be compatible with e.g. a universal fighting board.

This was originally developed for the purpose of adapting it to a Genesis 6-button pad, so the L trigger is not supported. There aren't enough pins on the uC anyway.
