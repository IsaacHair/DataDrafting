For best results, use DataDrafting_v2, InitSamples_v1, GenTrace_v2, and Truncate.

DataDrafting
  v0: allows for arbitrary input files that contain continuous binary data
  v1: same as v0 but deleted histograms and added simple compression algorithm
  v2: added extra tests; this is the most stable version
  v3-v5: perliminary tests for different reordering algorithms and new traces; not stable

InitSamples
  First version: initializes samples and randomly inserts zero bytes independently with probability 50%
  v1: initializes samples and randomly inserts longer strings of zeros

GenTrace
  v0: Takes a specific input format and uses a hash table to see what the cache blocks should be
  v1: Takes a specific input format and just concatenates reads together to fill up "cache blocks"
  v2: Record reads as part of what should be written in each cache block and made bug fixes

Truncate
  Simply cuts out part of a cache block access file and places it in another file
