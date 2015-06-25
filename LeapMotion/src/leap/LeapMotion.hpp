struct LeapData;

class LeapMotion
{
public:
    LeapMotion() : m_leapData(nullptr)
    {
    }

    ~LeapMotion();

    void RecalculateSkeleton();
    void OnRender();

    void Init();
    void Destroy();
private:
    LeapData* m_leapData;
};