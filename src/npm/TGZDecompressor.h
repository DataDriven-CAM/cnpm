#pragma once

#include <vector>
#include <sstream>
#include <ostream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <filesystem>
#include <functional>
#include <time.h>
#include <chrono>
#include <cmath>

#include "zlib.h"

#include "c_plus_plus_serializer.h"

namespace sylvanmats::npm{

/* values used in typeflag field */

#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */

/* GNU tar extensions */

#define GNUTYPE_DUMPDIR  'D'    /* file names from dumped directory */
#define GNUTYPE_LONGLINK 'K'    /* long link name */
#define GNUTYPE_LONGNAME 'L'    /* long file name */
#define GNUTYPE_MULTIVOL 'M'    /* continuation of file from another volume */
#define GNUTYPE_NAMES    'N'    /* file name that does not fit into main hdr */
#define GNUTYPE_SPARSE   'S'    /* sparse file */
#define GNUTYPE_VOLHDR   'V'    /* tape/volume header */


/* tar header */

#define BLOCKSIZE     512
#define SHORTNAMESIZE 100

struct tar_header
{                               /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
                                /* 500 */
};

union tar_buffer
{
  char               buffer[BLOCKSIZE];
  struct tar_header  header;
};
    enum { TGZ_EXTRACT, TGZ_LIST, TGZ_INVALID };
    
    class TGZDecompressor{
    protected:
    public:
        TGZDecompressor() = default;
        TGZDecompressor(const TGZDecompressor& orig) = delete;
        virtual ~TGZDecompressor() = default;

        void operator()(std::filesystem::path& filePath, std::function<void(std::filesystem::path& newPath, std::ostream& content)> apply){
//            if(filePath.string().rfind("~/", 0)==0){
//                const char* home = getenv("HOME");
//                if (home) {
//                    filePath=home+filePath.string().substr(1);
//                }
//            }
            if(std::filesystem::exists(filePath)){
                if(filePath.extension().compare(".tgz")==0){
                    std::stringstream ss;
                    gzFile file=gzopen(filePath.c_str(), "rb");
                    union  tar_buffer buffer;
                    int err;
                    int getheader = 1;
                    unsigned int entrySize = 0;
                    int remaining = 0;
                    char fname[BLOCKSIZE];
                    int tarmode;
                    time_t tartime;
                    int action = TGZ_EXTRACT;
                    std::filesystem::path newPath;
                    std::stringbuf dynBuff;
                    std::ostream os(&dynBuff);
                    do{
                        int len = gzread(file, &buffer, BLOCKSIZE);
                        if (len < 0)
                          std::cout<<gzerror(file, &err)<<std::endl;
                        if (getheader >= 1)
                          {
                            /*
                             * if we met the end of the tar
                             * or the end-of-tar block,
                             * we are done
                             */
                            if (len == 0 || buffer.header.name[0] == 0)
                              break;

                            tarmode = std::strtoul(std::string(buffer.header.mode, 0, 8).c_str(), nullptr, 8);
                            tartime = (time_t)std::strtoul(std::string(buffer.header.mtime, 0, 12).c_str(), nullptr, 8);
                            if (tarmode == -1 || tartime == (time_t)-1)
                              {
                                buffer.header.name[0] = 0;
                                action = TGZ_INVALID;
                              }

                            if (getheader == 1)
                              {
                                strncpy(fname,buffer.header.name,SHORTNAMESIZE);
                                if (fname[SHORTNAMESIZE-1] != 0)
                                    fname[SHORTNAMESIZE] = 0;
                              }
                            else
                              {
                                /*
                                 * The file name is longer than SHORTNAMESIZE
                                 */
                                if (strncmp(fname,buffer.header.name,SHORTNAMESIZE-1) != 0)
                                    std::cout<<"bad long name"<<std::endl;
                                getheader = 1;
                              }

                            /*
                             * Act according to the type flag
                             */
                            switch (buffer.header.typeflag)
                              {
                              case DIRTYPE:
                                if (action == TGZ_LIST)
                                  printf(" %s     <dir> %s\n",strtime(&tartime),fname);
                                if (action == TGZ_EXTRACT)
                                  {
                                    std::string filePath=fname;
                                    if(filePath.starts_with("package/"))filePath=filePath.substr(8, filePath.length()-8);
                                    newPath=filePath;
                                    //newPath
                                    apply(newPath, os);
//                                    makedir(fname);
                                    //push_attr(&attributes,fname,tarmode,tartime);
                                    newPath="";
                                    dynBuff=std::stringbuf();
                                    os.rdbuf(&dynBuff);
                                  }
                                break;
                              case REGTYPE:
                              case AREGTYPE:
                                remaining = std::strtoul(std::string(buffer.header.size, 0, 12).c_str(), nullptr, 8);
                                if (remaining == -1)
                                  {
                                    action = TGZ_INVALID;
                                    break;
                                  }
                                entrySize=remaining;
                                if (action == TGZ_LIST)
                                  printf(" %s %9d %s\n",strtime(&tartime),remaining,fname);
                                else if (action == TGZ_EXTRACT)
                                  {
//                                    if (matchname(arg,argc,argv,fname))
                                      {
                                            std::string filePath=fname;
                                            if(filePath.starts_with("package/"))filePath=filePath.substr(8, filePath.length()-8);
                                            newPath=filePath;
                                            dynBuff=std::stringbuf();
                                            os.rdbuf(&dynBuff);
//                                        outfile = fopen(fname,"wb");
//                                        if (outfile == NULL) {
//                                          /* try creating directory */
//                                          char *p = strrchr(fname, '/');
//                                          if (p != NULL) {
//                                            *p = '\0';
//                                            makedir(fname);
//                                            *p = '/';
//                                            outfile = fopen(fname,"wb");
//                                          }
//                                        }
//                                        if (outfile != NULL)
//                                          printf("Extracting %s\n",fname);
//                                        else
//                                          fprintf(stderr, "%s: Couldn't create %s",prog,fname);
                                      }
//                                    else
//                                      outfile = NULL;
                                  }
                                getheader = 0;
                                break;
                              case GNUTYPE_LONGLINK:
                              case GNUTYPE_LONGNAME:
                                remaining = std::strtoul(std::string(buffer.header.size, 0, 12).c_str(), nullptr, 8);
                                if (remaining < 0 || remaining >= BLOCKSIZE)
                                  {
                                    action = TGZ_INVALID;
                                    break;
                                  }
                                len = gzread(file, fname, BLOCKSIZE);
                                if (len < 0)
                                  std::cout<<gzerror(file, &err)<<std::endl;
                                if (fname[BLOCKSIZE-1] != 0 || (int)strlen(fname) > remaining)
                                  {
                                    action = TGZ_INVALID;
                                    break;
                                  }
                                getheader = 2;
                                break;
                              default:
                                if (action == TGZ_LIST)
                                  printf(" %s     <---> %s\n",strtime(&tartime),fname);
                                break;
                              }
                          }
                        else
                          {
                            unsigned int bytes = (remaining > BLOCKSIZE) ? BLOCKSIZE : remaining;

                            
                            if (!newPath.empty()){
                                for(unsigned int index=0;index<bytes;index++)os<<bits(buffer.buffer[index]);
                            }
                            remaining -= bytes;
                          }

                        if (remaining == 0)
                          {
                            getheader = 1;
                            if (!newPath.empty()){
                                apply(newPath, os);
                                newPath="";
                                dynBuff=std::stringbuf();
                                os.rdbuf(&dynBuff);
//                                fclose(outfile);
//                                outfile = NULL;
//                                if (action != TGZ_INVALID)
//                                  push_attr(&attributes,fname,tarmode,tartime);
                              }
                          }

                        /*
                         * Abandon if errors are found
                         */
                        if (action == TGZ_INVALID)
                          {
                            std::cout<<"broken archive"<<std::endl;
                            break;
                          }
                    }while(true);
                    gzclose(file);
//                    apply(ss);
                }
                else{
//                    std::ifstream file(filePath.c_str());
//                    //std::stringstream content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//                    apply(newPath, file);
//                    file.close();
                }
            }
        };

    protected:
        int octalToDecimal(int octalNumber)
        {
            int decimalNumber = 0, i = 0, rem;
            while (octalNumber != 0)
            {
                rem = octalNumber % 10;
                octalNumber /= 10;
                decimalNumber += rem * std::pow(8, i);
                ++i;
            }
            return decimalNumber;
        }
        
        char *strtime (time_t *t)
        {
          struct tm   *local;
          static char result[32];

          local = localtime(t);
          sprintf(result,"%4d/%02d/%02d %02d:%02d:%02d",
                  local->tm_year+1900, local->tm_mon+1, local->tm_mday,
                  local->tm_hour, local->tm_min, local->tm_sec);
          return result;
        }
    };
}

