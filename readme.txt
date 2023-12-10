DataDrafting_v0: allows for arbitrary input files that contain continuous binary data

DataDrafting_v1: same as v0 but deleted histograms and added simple compression algorithm

DataDrafting_v2: added extra tests

InitSamples: initializes samples and randomly inserts zero bytes independently with probability 50%

InitSamples_v1: initializes samples and randomly inserts longer strings of zeros

GenTrace_v0: Takes a specific input format and uses a hash table to see what the cache blocks should be

GenTrace_v1: Takes a specific input format and just concatenates reads together to fill up "cache blocks"

GenTrave_v2: Record reads as part of what should be written in each cache block and made bug fixes

Truncate: Simply cuts out part of a cache block access file and places it in another file
