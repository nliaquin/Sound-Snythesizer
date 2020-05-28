#include <iostream>
#include "NoiseMaker.h"

using namespace std;


double w(double dHertz)
{
    return dHertz * 2.0 * PI;
}


double osc(double dHertz, double dTime, int nType, double dLFOHertz = 0.0, double dLFOAmplitude = 0.0)
{
    double dFreq = w(dHertz) * dTime + dLFOAmplitude * dHertz * sin(w(dLFOHertz) * dTime);

    switch (nType)
    {
    case 1: //Sine Wave
        return sin(dFreq);

    case 2: //Square Wave
        return sin(dFreq) > 0.0 ? 1.0 : -1.0;

    case 3: //Triangle Wave
        return asin(sin(dFreq) * (2.0 / PI));

    case 4: //Analogue Saw Wave
    {
        double dOutput = 0.0;

        for (double n = 1.0; n < 40.0; n++)
            dOutput += (sin(n * dFreq)) / n;

        return dOutput * (2.0 / PI);
    }

    case 5: //Optimised Saw Wave
        return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));

    case 6: //Static
        return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;

    default:
        return 0.0;
    }
}


struct sEnvelopeADSR
{
    double dAttackTime;
    double dDecayTime;
    double dReleaseTime;

    double dSustainAmplitude;
    double dStartAmplitude;

    double dTriggerOnTime;
    double dTriggerOffTime;

    bool bNoteOn;

    sEnvelopeADSR()
    {
        dAttackTime = 0.01;
        dDecayTime = 1.0;
        dReleaseTime = 1.0;

        dSustainAmplitude = 0.0;
        dStartAmplitude = 1.0;

        dTriggerOnTime = 0.0;
        dTriggerOffTime = 0.0;

        bNoteOn = false;
    }

    void NoteOn(double dTimeOn)
    {
        dTriggerOnTime = dTimeOn;
        bNoteOn = true;
    }

    void NoteOff(double dTimeOff)
    {
        dTriggerOffTime = dTimeOff;
        bNoteOn = false;
    }

    double GetAmplitude(double dTime)
    {
        double dAmplitude = 0.0;
        double dLifeTime = dTime - dTriggerOnTime;

        if (bNoteOn)
        {
            //Attack
            if (dLifeTime <= dAttackTime)
                dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;

            //Decay
            if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
                dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

            //Sustain
            if (dLifeTime > (dAttackTime + dDecayTime))
            {
                dAmplitude = dSustainAmplitude;
            }
        }
        else
        {
            //Release
            dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude;
        }

        if (dAmplitude <= 0.0001)
        {
            dAmplitude = 0.0;
        }

        return dAmplitude;
    }
};


atomic<double> dFrequencyOutput = 0.0;
sEnvelopeADSR envelope;
double dOctaveBaseFrequency;
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);
int waveType1;
int waveType2;
double dLFHz;
double dLFAmp;


double MakeNoise(double dTime)
{
    //Cool low-pitch organ sound
    //double dOutput = envelope.GetAmplitude(dTime) * osc(dFrequencyOutput, dTime, waveType);

    double dOutput = envelope.GetAmplitude(dTime) *
        (
            + 1.0 * osc(dFrequencyOutput * 2.0, dTime, waveType1, dLFHz, dLFAmp)
            + 0.5 * osc(dFrequencyOutput * 3.0, dTime, waveType2)
            + 0.25 * osc(0, dTime, waveType1)
        );

    return dOutput * 0.9;
}


int main()
{
    vector<wstring> devices = NoiseMaker<short>::Enumerate();

    // if you want to list audio devices:
    //for (auto d : devices) wcout << "Found Output Devices: " << d << endl;

    wcout << "Enter wave type 1:" << endl;
    wcout << "0. None\n1. Sine Wave\n2. Square Wave\n3. Triangle Wave\n4. Analogue Saw Wave\n5. Optimized Sine Wave" << endl;
    cin >> waveType1;

    wcout << "\nEnter wave type 2:" << endl;
    wcout << "0. None\n1. Sine Wave\n2. Square Wave\n3. Triangle Wave\n4. Analogue Saw Wave\n5. Optimized Sine Wave" << endl;
    cin >> waveType2;

    wcout << "\nEnter the Octave Base Frequency (210 Recommended)(WARNING: DO NOT SET THIS TOO HIGH! You will blow your speakers out permanently.)" << endl;
    cin >> dOctaveBaseFrequency;

    wcout << "\nEnter low frequency Hz (enter 0 for none):" << endl;
    cin >> dLFHz;

    wcout << "\nEnter low frequency amplitude (enter 0 for none):" << endl;
    cin >> dLFAmp;

    //Print out the keys here:
    wcout << endl <<
        "|   |   |   |   |   | |   |   |   |   | |   | |   |   |     |" << endl <<
        "|   | S |   |   | F | | G |   |   | J | | K | | L |   |     |" << endl <<
        "|   |___|   |   |___| |___|   |   |___| |___| |___|   |     |" << endl <<
        "|     |     |     |     |     |     |     |     |     |     |" << endl <<
        "|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
        "|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

    NoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);
    
    sound.SetUserFunction(MakeNoise);

    int nCurrentKey = -1;
    bool keyPressed = false;

    while (1)
    {
        keyPressed = false;

        for (int k = 0; k < 16; k++)
        {
            if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe"[k])) & 0x8000)
            {
                if (nCurrentKey != k)
                {
                    dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
                    envelope.NoteOn(sound.GetTime());
                    nCurrentKey = k;
                }

                keyPressed = true;
            }
            
        }

        if (!keyPressed)
        {
            if (nCurrentKey != -1)
            {
                envelope.NoteOff(sound.GetTime());
                nCurrentKey = -1;
            }
        }
    }

    return 0;
}