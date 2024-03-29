#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

struct DataPoint
{
    int index;
    float value;
};

int main()
{
    char inputPath[255];
    cout << "Enter the input file name: ";
    cin >> inputPath;
    ifstream dataFile(inputPath);
    if (!dataFile.is_open())
    {
        cerr << "Error opening data file." << endl;
        return 1;
    }

    // Đọc tần số lấy mẫu
    string line;
    float sampleRate;
    getline(dataFile, line); // Bỏ qua dòng chứa "Sample Rate: XXXX Hz"
    getline(dataFile, line);
    size_t pos = line.find(" ");
    sampleRate = stof(line.substr(pos + 1));

    // Đọc dữ liệu từ file
    vector<DataPoint> dataPoints;
    while (getline(dataFile, line))
    {
        pos = line.find(":");
        int index = stoi(line.substr(pos + 2, line.find(",") - pos - 2));
        float value = stof(line.substr(line.find(":") + 2));
        dataPoints.push_back({index, value});
    }

    dataFile.close();

    // Gọi Gnuplot từ C++ thông qua pipes
    FILE *gnuplotPipe = popen("gnuplot -persist", "w");
    if (!gnuplotPipe)
    {
        cerr << "Error opening gnuplot pipe." << endl;
        return 1;
    }

    // Gửi lệnh đến Gnuplot để vẽ đồ

    fprintf(gnuplotPipe, "set title \"SndProg\" \n");

    string execute = "plot \"" + string(inputPath) + "\" skip 1 with lines\n";
    fprintf(gnuplotPipe, execute.c_str());
    fflush(gnuplotPipe);

    // Đóng pipes
    pclose(gnuplotPipe);

    return 0;
}
