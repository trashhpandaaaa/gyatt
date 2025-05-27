// Gyatt Configuration
class GyattConfig {
public:
    static const std::string VERSION;
    static const std::string AUTHOR;
    
    bool enableVerboseOutput = false;
    bool colorOutput = true;
};

const std::string GyattConfig::VERSION = "1.0.0";
const std::string GyattConfig::AUTHOR = "Gyatt Team";
