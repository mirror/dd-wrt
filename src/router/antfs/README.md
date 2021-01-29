# ANTFS
ANTFS - kernel mode driver based on NTFS-3G  

This driver supports reading/writing files with following main exceptions - You cannot:  
*write compressed files  
*access encrypted files  
*create links (reparse points/junctions)  
*use ACLs (everything is RWXRWXRWX)  
