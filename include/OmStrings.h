#ifndef OMSTRINGS_H_INCLUDED
#define OMSTRINGS_H_INCLUDED

// Generic name and validation
#define OMM_STR_ERR_ISDIR       L" does not exist or is not a directory."
#define OMM_STR_ERR_ISFILEZIP   L" is not a valid ZIP archive."
#define OMM_STR_ERR_VALIDNAME   L" cannot contain the following characters: / * ? \" < > | \\"
#define OMM_STR_ERR_VALIDPATH   L" cannot contain the following characters: / * ? \" < > |"
#define OMM_STR_ERR_VALIDURL    L" is not a valid HTTP(S) URL."
#define OMM_STR_ERR_CREATE(e)   L" cannot be created: " + e
#define OMM_STR_ERR_DELETE(e)   L" cannot be deleted: " + e
#define OMM_STR_ERR_RENAME(e)   L" cannot be renamed: " + e
#define OMM_STR_ERR_MOVE(e)     L" cannot be moved: " + e
#define OMM_STR_ERR_COPY(e)     L" cannot be copied: " + e
#define OMM_STR_ERR_READ        L" read permission denied."
#define OMM_STR_ERR_WRITE       L" write permission denied."

// Generic config errors
#define OMM_STR_ERR_DEFINIT(e)  L" cannot be initialized: " + e
#define OMM_STR_ERR_DEFOPEN(e)  L" cannot be loaded: " + e
#define OMM_STR_ERR_DEFSAVE(e)  L" cannot be saved: " + e
#define OMM_STR_ERR_ZIPINIT(e)  L" cannot be initialized: " + e
#define OMM_STR_ERR_ZIPOPEN(e)  L" cannot be loaded: " + e
#define OMM_STR_ERR_ZIPDEFL(e)  L" cannot be added or created into archive: " + e
#define OMM_STR_ERR_ZIPINFL(e)  L" cannot be extracted: " + e
#define OMM_STR_ERR_DIRACCESS   L" cannot be accessed, either it does not exist or have permission restriction."

// Generic dialog queries
#define OMM_STR_QRY_OVERWRITE   L" already exists, do you want to overwrite it ?"

#endif // OMSTRINGS_H_INCLUDED
