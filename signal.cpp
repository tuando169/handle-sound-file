#include <iostream>
#include <vector>
#include <sndfile.h>
#include <fstream>
#include <cmath>

using namespace std;

class Signal
{
public:
    vector<float> valueList; // Dãy giá trị âm thanh
    vector<int> indexList;   // Dãy chỉ số
    float sampleRate;        // Tần số lấy mẫu

    Signal(vector<float> &inputValueList,
           vector<int> &inputIndexList,
           float inputSampleRate)
        : valueList(inputValueList), indexList(inputIndexList), sampleRate(inputSampleRate) {}

    // Phương thức để hiển thị thông tin về tín hiệu âm thanh
    void displayInfo()
    {
        cout << "Index - Value pairs:" << endl;
        for (int i = 0; i < min(indexList.size(), valueList.size()); ++i)
        {
            cout << "Index: " << indexList[i] << ", Value: " << valueList[i] << endl;
        }
        cout << endl;
        cout << "Sample Rate: " << sampleRate << " Hz" << endl;
    }

    void timeShift(int shiftAmount)
    {
        vector<float> shiftedValueList;
        vector<int> shiftedIndexList;
        for (int i = 0; i < valueList.size(); i++)
        {
            int newIndex = indexList[i] - shiftAmount;
            shiftedValueList.push_back(valueList[i]);
            shiftedIndexList.push_back(newIndex);
        }

        cout << "Shifted Index Array: ";
        for (auto index : shiftedIndexList)
        {
            cout << index << " ";
        }
        cout << endl;

        cout << "Shifted Value Array: ";
        for (auto value : shiftedValueList)
        {
            cout << value << " ";
        }
        cout << endl;
    }

    void timeReversal()
    {
        vector<float> reversalValueList;
        vector<int> reversalIndexList;
        for (int i = valueList.size() - 1; i >= 0; i--)
        {
            reversalValueList.push_back(valueList[i]);
            reversalIndexList.push_back(-indexList[i]);
        }

        cout << "Reversal Index Array: ";
        for (auto index : reversalIndexList)
        {
            cout << index << " ";
        }
        cout << endl;

        cout << "Reversal Value Array: ";
        for (auto value : reversalValueList)
        {
            cout << value << " ";
        }
        cout << endl;
    }

    Signal add(Signal &other)
    {
        // Kiểm tra kích thước của hai dãy
        if (valueList.size() != other.valueList.size() || indexList.size() != other.indexList.size())
        {
            cout << "Error: Dãy không cùng kích thước." << endl;
        }
        else
        {
            vector<float> sumValueList;
            for (int i = 0; i < valueList.size(); i++)
            {
                sumValueList.push_back(valueList[i] + other.valueList[i]);
            }

            return Signal(sumValueList, indexList, sampleRate);
        }
    }

    Signal multiply(Signal &other)
    {
        // Kiểm tra kích thước của hai dãy
        if (valueList.size() != other.valueList.size() || indexList.size() != other.indexList.size())
        {
            cout << "Error: Dãy không cùng kích thước." << endl;
        }
        else
        {
            vector<float> productValueList;
            for (int i = 0; i < valueList.size(); i++)
            {
                productValueList.push_back(valueList[i] * other.valueList[i]);
            }

            return Signal(productValueList, indexList, sampleRate);
        }
    }

    Signal multiplyByScalar(float scalar)
    {
        vector<float> scaledValueList;
        for (float value : valueList)
        {
            scaledValueList.push_back(value * scalar);
        }

        return Signal(scaledValueList, indexList, sampleRate);
    }

    Signal downsample(int m)
    {
        vector<float> downsampledValueList;
        vector<int> downsampledIndexList;

        for (int i = 0; i < valueList.size(); i += m)
        {
            downsampledValueList.push_back(valueList[i]);
            downsampledIndexList.push_back(indexList[i]);
        }

        return Signal(downsampledValueList, downsampledIndexList, sampleRate / m);
    }

    Signal upsample(int l)
    {
        vector<float> upsampledValueList;
        vector<int> upsampledIndexList;

        for (int i = 0; i < valueList.size(); i++)
        {
            upsampledValueList.push_back(valueList[i]);

            upsampledIndexList.push_back(upsampledValueList.size());
            // Nội suy giá trị 0 giữa các mẫu
            for (int j = 1; j < l; j++)
            {
                upsampledValueList.push_back(0);
            }
            upsampledIndexList.push_back(upsampledValueList.size());
        }

        return Signal(upsampledValueList, upsampledIndexList, sampleRate * l);
    }

    Signal lowPassFilter(int filterLength, float cutoffFreq)
    {
        vector<float> filteredValues;

        // Tạo bộ lọc thông thấp FIR
        vector<float> filterCoefficients(filterLength, 0.0);
        int middleIndex = filterLength / 2;
        for (int i = 0; i < filterLength; ++i)
        {
            // Tính toán hệ số cho bộ lọc
            if (i == middleIndex)
            {
                filterCoefficients[i] = 2 * cutoffFreq / sampleRate;
            }
            else
            {
                float diff = (i - middleIndex) * 1.0;
                filterCoefficients[i] = sin(2 * M_PI * cutoffFreq * diff / sampleRate) / (M_PI * diff);
            }
        }

        // Áp dụng bộ lọc cho từng mẫu
        int dataSize = valueList.size();
        for (int i = 0; i < dataSize; ++i)
        {
            float filteredValue = 0.0;
            for (int j = 0; j < filterLength; ++j)
            {
                int index = i - middleIndex + j;
                if (index >= 0 && index < dataSize)
                {
                    filteredValue += filterCoefficients[j] * valueList[index];
                }
            }
            filteredValues.push_back(filteredValue);
        }

        // Trả về một đối tượng Signal mới với dãy giá trị âm thanh đã lọc
        return Signal(filteredValues, indexList, sampleRate);
    }

    Signal bandPassFilter(int filterLength, float lowCutoffFreq, float highCutoffFreq)
    {
        vector<float> filteredValues;

        // Tạo bộ lọc thông dải FIR
        vector<float> filterCoefficients(filterLength, 0.0);
        int middleIndex = filterLength / 2;
        float nyquistFreq = sampleRate / 2.0;
        for (int i = 0; i < filterLength; ++i)
        {
            // Tính toán hệ số cho bộ lọc
            float diff = (i - middleIndex) * 1.0;
            float coeff;
            if (diff == 0)
            {
                coeff = 2 * (highCutoffFreq - lowCutoffFreq) / sampleRate;
            }
            else
            {
                coeff = (sin(2 * M_PI * highCutoffFreq * diff / sampleRate) - sin(2 * M_PI * lowCutoffFreq * diff / sampleRate)) / (M_PI * diff);
            }
            filterCoefficients[i] = coeff;
        }

        // Áp dụng bộ lọc cho từng mẫu
        int dataSize = valueList.size();
        for (int i = 0; i < dataSize; ++i)
        {
            float filteredValue = 0.0;
            for (int j = 0; j < filterLength; ++j)
            {
                int index = i - middleIndex + j;
                if (index >= 0 && index < dataSize)
                {
                    filteredValue += filterCoefficients[j] * valueList[index];
                }
            }
            filteredValues.push_back(filteredValue);
        }

        // Trả về một đối tượng Signal mới với dãy giá trị âm thanh đã lọc
        return Signal(filteredValues, indexList, sampleRate);
    }

    Signal highPassFilter(int filterLength, float cutoffFreq)
    {
        vector<float> filteredValues;

        // Tạo bộ lọc thông cao FIR
        vector<float> filterCoefficients(filterLength, 0.0);
        int middleIndex = filterLength / 2;
        for (int i = 0; i < filterLength; ++i)
        {
            // Tính toán hệ số cho bộ lọc
            if (i == middleIndex)
            {
                filterCoefficients[i] = 1 - 2 * cutoffFreq / sampleRate;
            }
            else
            {
                float diff = (i - middleIndex) * 1.0;
                filterCoefficients[i] = -sin(2 * M_PI * cutoffFreq * diff / sampleRate) / (M_PI * diff);
            }
        }

        // Áp dụng bộ lọc cho từng mẫu
        int dataSize = valueList.size();
        for (int i = 0; i < dataSize; ++i)
        {
            float filteredValue = 0.0;
            for (int j = 0; j < filterLength; ++j)
            {
                int index = i - middleIndex + j;
                if (index >= 0 && index < dataSize)
                {
                    filteredValue += filterCoefficients[j] * valueList[index];
                }
            }
            filteredValues.push_back(filteredValue);
        }

        // Trả về một đối tượng Signal mới với dãy giá trị âm thanh đã lọc
        return Signal(filteredValues, indexList, sampleRate);
    }

    Signal bandStopFilter(int filterLength, float cutoffFreqLow, float cutoffFreqHigh)
    {
        vector<float> filteredValues;

        // Tạo bộ lọc cấm dải FIR
        vector<float> filterCoefficients(filterLength, 0.0);
        int middleIndex = filterLength / 2;
        for (int i = 0; i < filterLength; ++i)
        {
            // Tính toán hệ số cho bộ lọc
            if (i == middleIndex)
            {
                filterCoefficients[i] = 1.0 - 2.0 * (cutoffFreqHigh - cutoffFreqLow) / sampleRate;
            }
            else
            {
                float diff = (i - middleIndex) * 1.0;
                filterCoefficients[i] = (sin(2.0 * M_PI * cutoffFreqLow * diff / sampleRate) - sin(2.0 * M_PI * cutoffFreqHigh * diff / sampleRate)) / (M_PI * diff);
            }
        }

        // Áp dụng bộ lọc cho từng mẫu
        int dataSize = valueList.size();
        for (int i = 0; i < dataSize; ++i)
        {
            float filteredValue = 0.0;
            for (int j = 0; j < filterLength; ++j)
            {
                int index = i - middleIndex + j;
                if (index >= 0 && index < dataSize)
                {
                    filteredValue += filterCoefficients[j] * valueList[index];
                }
            }
            filteredValues.push_back(filteredValue);
        }

        // Trả về một đối tượng Signal mới với dãy giá trị âm thanh đã lọc
        return Signal(filteredValues, indexList, sampleRate);
    }

    Signal addFadeIn(float duration)
    {
        std::vector<float> fadedValues = valueList;
        int numSamples = fadedValues.size();
        float fadeIncrement = 1.0 / (duration * sampleRate);
        float currentGain = 0.0;

        for (int i = 0; i < numSamples; ++i)
        {
            fadedValues[i] *= currentGain;
            currentGain += fadeIncrement;
            if (currentGain > 1.0)
                currentGain = 1.0;
        }

        return Signal(fadedValues, indexList, sampleRate);
    }

    Signal addFadeOut(float duration)
    {
        std::vector<float> fadedValues = valueList;
        int numSamples = fadedValues.size();
        float fadeDecrement = 1.0 / (duration * sampleRate);
        float currentGain = 1.0;

        for (int i = numSamples - 1; i >= 0; --i)
        {
            fadedValues[i] *= currentGain;
            currentGain -= fadeDecrement;
            if (currentGain < 0.0)
                currentGain = 0.0;
        }

        return Signal(fadedValues, indexList, sampleRate);
    }

    Signal addEcho(Signal &sound, float delay, float decay)
    {
        int echoSamples = static_cast<int>(delay * sound.sampleRate);
        std::vector<float> echoBuffer(echoSamples, 0.0);

        // Tạo một bản sao của tín hiệu âm thanh ban đầu
        Signal echoedSound = sound;

        // Tính toán và thêm echo vào tín hiệu sao chép
        for (int i = 0; i < echoedSound.valueList.size(); ++i)
        {
            if (i >= echoSamples)
            {
                echoedSound.valueList[i] += echoBuffer[i % echoSamples] * decay;
            }
            echoBuffer[i % echoSamples] = echoedSound.valueList[i];
        }

        return echoedSound;
    }

    Signal applyReverb(vector<float> &impulseResponse)
    {
        int numSamples = valueList.size();
        int irLength = impulseResponse.size();
        vector<float> output(numSamples + irLength - 1, 0.0);

        // Thực hiện convolution giữa tín hiệu âm thanh và impulse response
        for (int n = 0; n < numSamples; ++n)
        {
            for (int k = 0; k < irLength; ++k)
            {
                output[n + k] += valueList[n] * impulseResponse[k];
            }
        }

        // Tạo một đối tượng Signal mới từ output và trả về
        return Signal(output, indexList, sampleRate);
    }

    Signal addFlanging(float depth, float rate, float delayTime)
    {
        int numSamples = valueList.size();
        vector<float> flangedValues(numSamples, 0.0);
        vector<float> delayedSignal(numSamples, 0.0);

        // Tạo ra sự chậm trễ
        int delaySamples = delayTime * sampleRate;
        for (int i = delaySamples; i < numSamples; ++i)
        {
            delayedSignal[i] = valueList[i - delaySamples];
        }

        // Tạo ra biến thiên trong thời gian
        float phaseIncrement = rate / sampleRate;
        float phase = 0.0;
        for (int i = 0; i < numSamples; ++i)
        {
            float modAmount = depth * sin(2 * M_PI * phase);
            flangedValues[i] = valueList[i] + modAmount * delayedSignal[i];
            phase += phaseIncrement;
            if (phase > 1.0)
                phase -= 1.0;
        }

        return Signal(flangedValues, indexList, sampleRate);
    }
};
int main()
{
    char inputPath[255];
    cout << "Enter the input file name: ";
    cin >> inputPath;

    char *inputFile = inputPath;
    SF_INFO sfInfo;
    SNDFILE *sndfile = sf_open(inputFile, SFM_READ, &sfInfo);

    if (!sndfile)
    {
        cerr << "Error opening input file: " << inputFile << endl;
        return 1;
    }

    // Đọc dữ liệu từ file WAV và lưu vào vector
    vector<float> audioValueList;
    vector<int> audioIndexList;
    float sampleRate = sfInfo.samplerate;
    float value;
    for (int i = 0; sf_read_float(sndfile, &value, 1) > 0; ++i)
    {
        audioValueList.push_back(value);
        audioIndexList.push_back(i);
    }
    Signal inputSignal = Signal(audioValueList, audioIndexList, sampleRate);

    // Function Selections
    // while (true)
    // {
    //     cout << "-----------------------------------";
    //     cout << "1. Display Infomation" << endl;
    //     cout << "2. Time Shifting" << endl;
    //     cout << "3. Time Reversal" << endl;
    //     cout << "4. Add" << endl;
    //     cout << "5. Multiply" << endl;
    //     cout << "6. Multiply By Scalar" << endl;
    //     cout << "7. Downsample" << endl;
    //     cout << "8. Upsample" << endl;
    //     cout << "9. Low Pass Filter" << endl;
    //     cout << "10. Band Pass Filter" << endl;
    //     cout << "11. High Pass Filter" << endl;
    //     cout << "12. High Stop Filter" << endl;
    //     cout << "-----------------------------------";
    //     int t;
    //     cin >> t;
    //     switch (t)
    //     {
    //     case 1:
    //         inputSignal.displayInfo();
    //         break;

    //     case 2:
    //         cout << "Enter Shift Amount: ";
    //         int shiftAmount;
    //         cin >> shiftAmount;
    //         inputSignal = inputSignal.timeShift(shiftAmount);
    //         break;

    //     case 3:
    //         inputSignal = inputSignal.timeReversal();
    //         break;

    //     case 4:
    //     cout << ""
    //         int n;
    //         cin >> n;

    //         vector<int> anotherIndexList;
    //         vector<float> anotherValueList;
    //         for (int i = 0; i < n; i++)
    //         {
    //             anotherIndexList.push_back(i);
    //             float x;
    //             cin >> x;
    //             anotherValueList.push_back(x);
    //         }
    //         float anotherSampleRate;

    //         break;

    //     case 5:
    //         break;

    //     case 6:
    //         break;

    //     default:
    //         break;
    //     }
    // }

    // Low Pass Filter
    float cutoffFreq = 500.0; // Tần số cắt
    int filterLength = 10;    // Độ dài của bộ lọc
    Signal outputSignal = inputSignal.lowPassFilter(filterLength, cutoffFreq);

    // Fade
    // float duraton = 1;

    // Signal outputSignal = inputSignal.addFadeIn(duraton);

    //  Lưu dữ liệu vào file data.txt
    vector<int> outputIndexList = outputSignal.indexList;
    vector<float> outputValueList = outputSignal.valueList;
    float outputSampleRate = outputSignal.sampleRate;

    // Ghi dữ liệu âm thanh vào file
    char outputPath[] = "data.txt";
    // cout << "Enter the output file name: ";
    // cin >> outputPath;
    ofstream outputFile(outputPath);
    if (outputFile.is_open())
    {
        outputFile << outputSampleRate << endl;
        for (int i = 0; i < outputValueList.size(); ++i)
        {
            outputFile << outputIndexList[i] << " " << outputValueList[i] << endl;
        }

        outputFile.close();
        cout << "Data has been saved." << endl;
    }
    else
    {
        cout << "Error opening the file." << endl;
    }
    return 0;
}