#include "SHA2.h"

/* Constructor */
SHA2::SHA2() { digest.reset(new unsigned char[digestor.DIGESTSIZE]); }

/* Destructor */
SHA2::~SHA2() {}

/* Input overall data to be processed */
void SHA2::inputData(char *data, size_t dataLength)
{
    digestor.Update((unsigned char *)data, dataLength);
}

/* Input (repetitively) partial data to be processed */
void SHA2::inputDataPart(char *data, size_t dataLength)
{
    digestor.Update((unsigned char *)data, dataLength);
}

/* Compute unique identifier for input data and convert it to Hex format */
std::string SHA2::hashData()
{
    digestor.Final(digest.get());

    std::string output;
    encoder.Attach(new CryptoPP::StringSink(output));
    encoder.Put(digest.get(), digestor.DIGESTSIZE);
    encoder.MessageEnd();
    encoder.Detach();
    return output;
}
