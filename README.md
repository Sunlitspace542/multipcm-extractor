# SEGA MultiPCM Sample Extractor
For extracting samples from SEGA MultiPCM sample ROMs.  
Tested with Daytona USA and Virtua Racing.  
Fork of [m2me](https://github.com/belaw/m2me) by belaw.  
  
To use: ``m2me [options]`` (Or just drag and drop!)  

Options:  
```
-i[infile]    Specify input ROM (Required!)
-o[outdir]    Specify output directory (Optional, no effect when used with -t)
-t[infile]    Test mode (No files written)
-s[num]       Extract samples starting from this instrument table entry (0 - ...)
-e[num]       Extract up to this number of samples (Must be nonzero)
-h, -help     Show help text
```