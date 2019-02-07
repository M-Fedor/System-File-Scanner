#ifndef OutputDBConnection_h
#define OutputDBConnection_h

#define DIGEST_SIZE 65
#define NAME_SIZE 300
#define VERSION_SIZE 200

#include "Output.h"
#include <cstring>
#include <mysql/mysql.h>
#include <vector>

/* Class implements communication with MariaDB/MySQL DBMS
in order to determine output and outputs data in output file */
class OutputDBConnection : public Output
{
public:
  OutputDBConnection(char *hostName, char *userName, char *userPasswd,
                     char *dbName, unsigned int portNum, char *unixSocket);
  ~OutputDBConnection();

  int init();
  int outputData(std::string digest, std::string name);

private:
  void resizeBuffers();
  void setBind(
      MYSQL_BIND &bind, enum enum_field_types field_type, void *param, size_t paramSize,
      size_t *paramLen, my_bool &isNull, my_bool &error, char &ind);

  char *dbName;
  char *fileDigest;
  char *fileName;
  char *fileVersion;
  char *getDigestFileNameStr;
  char *hostName;
  char *unixSocket;
  char *userName;
  char *userPasswd;
  int bufferSizeCoefficient;
  MYSQL *mysql;
  MYSQL_BIND bind[5];
  MYSQL_STMT *getDigestFileName;
  MYSQL_TIME fileCreated;
  MYSQL_TIME fileChanged;
  std::vector<my_bool> error;
  std::vector<my_bool> isNull;
  std::vector<size_t> paramLen;
  unsigned int portNum;
};

#endif