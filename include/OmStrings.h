#ifndef OMSTRINGS_H_INCLUDED
#define OMSTRINGS_H_INCLUDED

// Generic name and validation
#define OMM_STR_ERR_ISDIR(s)      L"The folder \"" + s + L"\" does not exists or is not a directory"
#define OMM_STR_ERR_ISFILEZIP(s)  L"The file \"" + s + L"\" si not a valid ZIP archive"
#define OMM_STR_ERR_VALIDNAME     L"Name cannot contains the following characters: / * ? \" < > | \\"
#define OMM_STR_ERR_VALIDPATH     L"Path cannot contains the following characters: / * ? \" < > |"

#endif // OMSTRINGS_H_INCLUDED
