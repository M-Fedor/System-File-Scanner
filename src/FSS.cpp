#include "FSS.h"

int executeInFileMode()
{
    std::shared_ptr<InputFile> inFile(new InputFile(inputFileName.data(), regexTarget.data()));
    std::vector<std::shared_ptr<Output>> outputList;
    std::shared_ptr<OutputOffline> out(new OutputOffline(outputFileName.data()));
    for (unsigned int i = 0; i < nCores; i++)
        outputList.push_back(std::shared_ptr<Output>(new OutputDBConnection(
            out.get(), hostName.data(), userName.data(), password.data(), dbName.data(), dbPort, NULL)));
    std::shared_ptr<ParallelExecutor> exec =
        (errFileName.size() == 0) ? std::shared_ptr<ParallelExecutor>(new ParallelExecutor(inFile, outputList))
                                  : std::shared_ptr<ParallelExecutor>(new ParallelExecutor(inFile, outputList, errFileName.data()));
    if (exec->init())
        return 1;
    exec->validate();

    return 0;
}

int executeInScannerMode()
{
    std::shared_ptr<InputScanner> inScanner(new InputScanner(rootDirectories, regexTarget.data()));
    std::vector<std::shared_ptr<HashAlgorithm>> hashAlgList;
    std::vector<std::shared_ptr<Output>> outputList;
    std::shared_ptr<OutputOffline> out(new OutputOffline(outputFileName.data()));
    std::shared_ptr<ParallelExecutor> exec;
    for (unsigned int i = 0; i < nCores; i++)
        hashAlgList.push_back(std::shared_ptr<HashAlgorithm>(new SHA2()));
    if (offline)
        exec = (errFileName.size() == 0)
                   ? std::shared_ptr<ParallelExecutor>(new ParallelExecutor(inScanner, hashAlgList, out))
                   : std::shared_ptr<ParallelExecutor>(new ParallelExecutor(inScanner, hashAlgList, out, errFileName.data()));
    else
    {
        for (unsigned int i = 0; i < nCores; i++)
            outputList.push_back(std::shared_ptr<OutputDBConnection>(new OutputDBConnection(
                out.get(), hostName.data(), userName.data(), password.data(), dbName.data(), dbPort, NULL)));
        exec = (errFileName.size() == 0)
                   ? std::shared_ptr<ParallelExecutor>(new ParallelExecutor(inScanner, hashAlgList, outputList))
                   : std::shared_ptr<ParallelExecutor>(new ParallelExecutor(inScanner, hashAlgList, outputList, errFileName.data()));
    }

    if (exec->init())
        return 1;
    exec->validate();

    return 0;
}

void getInputOpt()
{
    if (inputFile)
    {
        do
        {
            (std::cout << "Path to input file []: ").flush();
            std::getline(std::cin, inputFileName);
        } while (inputFileName.size() == 0);
        offline = false;
    }
    else
    {
        std::string rootDir;
        do
        {
            do
            {
                (std::cout << "Root-of-search directory []: ").flush();
                std::getline(std::cin, rootDir);
            } while (rootDir.size() == 0);
            rootDirectories.push_back(std::string(rootDir));
            do
            {
                (std::cout << "Want to specify another directory? [y/Y]: ").flush();
                std::getline(std::cin, rootDir);
            } while (rootDir.size() == 0);
        } while (!strcmp(rootDir.data(), "Y") || !strcmp(rootDir.data(), "y") ||
                 !strcmp(rootDir.data(), "yes"));
    }
    (std::cout << "Regular expression for input filtering [.*]: ").flush();
    std::getline(std::cin, regexTarget);
    if (regexTarget.size() == 0)
        regexTarget = std::string(".*");
    std::cout << "\n";
}

void getOutputOpt()
{
    std::string value;
    if (offline)
    {
        (std::cout << "Name of output file [Offline_scan.txt]: ").flush();
        std::getline(std::cin, outputFileName);
        if (outputFileName.size() == 0)
            outputFileName = std::string("Offline_scan.txt");
    }
    else
    {
        (std::cout << "Name of output file [Validation_results.txt]: ").flush();
        std::getline(std::cin, outputFileName);
        if (outputFileName.size() == 0)
            outputFileName = std::string("Validation_results.txt");

        (std::cout << "Database hostname [localhost]: ").flush();
        std::getline(std::cin, hostName);
        if (hostName.size() == 0)
            hostName = std::string("localhost");

        (std::cout << "Database port [3306]: ").flush();
        std::getline(std::cin, value);
        int rc = std::atoi(value.data());
        dbPort = (value.size() == 0 || rc <= 0) ? 3306 : rc;

        (std::cout << "Database name [test]: ").flush();
        std::getline(std::cin, dbName);
        if (dbName.size() == 0)
            dbName = std::string("test");

        (std::cout << "Database username [root]: ").flush();
        std::getline(std::cin, userName);
        if (userName.size() == 0)
            userName = std::string("root");

        (std::cout << "Database password: ").flush();
        secureInput(password);
    }

    (std::cout << "Name of error-output file []: ").flush();
    std::getline(std::cin, errFileName);

    (std::cout << "Number of jobs run in parallel ["
               << 2 * std::thread::hardware_concurrency() << "]: ")
        .flush();
    std::getline(std::cin, value);
    int rc = std::atoi(value.data());
    nCores = (value.size() == 0 || rc <= 0) ? 2 * std::thread::hardware_concurrency() : rc;
    std::cout << "\n";
}

int main(int argc, char **args)
{
    int rc = resolveOptions(argc, args);
    if (rc != 0)
        return rc > 0 ? 0 : 1;

    getInputOpt();
    getOutputOpt();
    std::cout << "Initializing...\n";

    if (inputFile)
        return executeInFileMode();
    else
        return executeInScannerMode();
}

void printHelp() {}

void printVersion() {}

int resolveOptions(int argc, char **args)
{
    const char *short_options = "fhoVv";
    struct option long_options[] = {{"file", 0, NULL, 'f'},
                                    {"help", 0, NULL, 'h'},
                                    {"offline", 0, NULL, 'o'},
                                    {"verbose", 0, NULL, 'V'},
                                    {"version", 0, NULL, 'v'},
                                    {NULL, 0, NULL, 0}};

    char nextOption = getopt_long(argc, args, short_options, long_options, NULL);
    while (nextOption != -1)
    {
        switch (nextOption)
        {
        case 'f':
            inputFile = true;
            break;
        case 'h':
            printHelp();
            return 1;
        case 'o':
            offline = true;
            break;
        case 'V':
            verbose = true;
            break;
        case 'v':
            printVersion();
            return 1;
        default:
            printHelp();
            return -1;
        }
        nextOption = getopt_long(argc, args, short_options, long_options, NULL);
    }
    return 0;
}

void secureInput(std::string &input)
{
    termios oldOpt;
    tcgetattr(STDIN_FILENO, &oldOpt);
    termios newOpt = oldOpt;
    newOpt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newOpt);

    std::getline(std::cin, input);
    std::cout << "\n";
    tcsetattr(STDIN_FILENO, TCSANOW, &oldOpt);
}