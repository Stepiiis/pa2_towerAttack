#pragma once
/// structure of effects used to pass effect values.
struct CEffects{
    explicit CEffects(int slowEffect = 0, int fireEffect = 0): m_slowEffect(slowEffect), m_fireEffect(fireEffect){}
    CEffects(const CEffects & rhs) = default;
    CEffects & operator=(const CEffects & rhs) = default;
    CEffects & operator+=(const CEffects & rhs) {
        m_slowEffect += rhs.m_slowEffect;
        m_fireEffect += rhs.m_fireEffect;
        return *this;
    }
    CEffects & operator-=(const CEffects & rhs) {
        m_slowEffect -= rhs.m_slowEffect;
        m_fireEffect -= rhs.m_fireEffect;
        return *this;
    }
    int m_slowEffect{};
    int m_fireEffect{};
    // to be added
};
/// terminal helper structure which is used for window size
struct terminal{
    int height{};
    int width{};
};