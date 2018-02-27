#include <string>
#include <fstream>
#include <cassert>
#include <vector>

struct Literal
{
    int32_t variable;
    bool negated;
};

class Tracer
{
public:
    Tracer(const std::string & traceFile, const std::string & simplifiedFile, const std::string & simplifiedHeader)
    {
        setTraceFile(traceFile);
        setSimplifiedFile(simplifiedFile, simplifiedHeader);
    }

    ~Tracer()
    {
        writeHeader(false, m_currentRestarts);
    }


    void inline traceBacktrack(int32_t level) { trace('<', level); }
    void inline traceNewDecisionLevel(int32_t level) { trace('>', level); }
    void inline traceBranch(const Literal & literal) { traceLiteral('B', literal); }
    void inline traceSetVariable(const Literal & literal) { traceLiteral('+', literal); }
    void inline traceConflict(const Literal & literal) { traceLiteral('C', literal); }
    void inline traceRestart() { trace('R', m_currentRestarts); ++m_currentRestarts; }

    void inline traceLearntClause(int32_t clause_id, const std::vector<Literal> & clause)
    {
        trace('L', clause_id);
        trace('S', clause.size());
        for(const auto & literal : clause) {
            traceLiteral('x', literal);
        }
    }

    void inline traceUnlearntClause(int32_t clause_id)
    {
        trace('U', clause_id);
    }

private:

    void inline traceLiteral(char label, const Literal & literal)
    {
        trace(label, literal.negated ? (-literal.variable) : literal.variable);
    }

    void inline trace(char label, int32_t data)
    {
        if (label == '>')
        {
            assert(data == levelForAssert + 1);
            levelForAssert = data;
        }
        if (label == '<')
        {
            assert(data < levelForAssert || data == 0);
            levelForAssert = data;
        }
        traceFile.write(&label, 1);
        traceFile.write(reinterpret_cast<const char *>(&data), sizeof(data));
    }

    template<typename T>
    void writeToHeader(std::ofstream & file, const bool dryRun, const T & data, int32_t & position)
    {
        if (!dryRun)
        {
            file.seekp(position);
            file.write(reinterpret_cast<const char *>(&data), sizeof(data));
        }
        position += sizeof(data);
    }

    int32_t inline writeHeader(const bool dryRun, const int32_t numberOfRestarts)
    {
        if (!dryRun)
        {
            traceFile.close();
            traceFile.open(m_name, std::ios::binary | std::ios::out | std::ios::in);
        }
        int32_t headerSize;
        headerSize = sizeof(headerSize) + sizeof(numberOfRestarts);
        if (headerSize % 5 != 0)
        {
            headerSize += 5 - (headerSize % 5);
        }
        int32_t position = 0;
        writeToHeader(traceFile, dryRun, headerSize, position);
        writeToHeader(traceFile, dryRun, numberOfRestarts, position);
        assert(headerSize >= position);
        if (!dryRun)
        {
            assert(position == traceFile.tellp());
            traceFile.close();
        }
        return headerSize;
    }
    void inline writeDummyHeader()
    {
        const auto headerSize = writeHeader(true, -1);
        const char c = 0;
        for (int32_t i = 0; i < headerSize; ++i)
        {
            traceFile.write(&c, 1);
        }
    }

    void inline setSimplifiedFile(const std::string & name, const std::string & header)
    {
        simplifiedFile.open(name);
        simplifiedFile << header;
    }

    void inline setTraceFile(const std::string & name)
    {
        m_name = name;
        traceFile.open(name, std::ios::binary | std::ios::out);
        writeDummyHeader();
    }

    std::ofstream traceFile;
    std::ofstream simplifiedFile;
    std::string m_name;
    int levelForAssert = 0;
    int m_currentRestarts = 0;
};
