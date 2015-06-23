struct LeapData;

class LeapMotion
{
public:
    LeapMotion() : m_leapData(nullptr)
    {
    }

    ~LeapMotion();

    void Init();
    void Destroy();
private:
    LeapData* m_leapData;
};