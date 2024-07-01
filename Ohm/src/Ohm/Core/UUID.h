#pragma once

namespace Ohm
{
    class UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_UUID; }
    private:
        uint64_t m_UUID;
    };
}

namespace std
{
    template <typename T> struct hash;

    template<>
    struct hash<Ohm::UUID>
    {
        std::size_t operator()(const Ohm::UUID& uuid) const noexcept
        {
            return uuid;
        }
    };

}
