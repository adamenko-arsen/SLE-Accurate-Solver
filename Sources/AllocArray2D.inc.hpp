template<typename T>
class AllocArray2D
{
public:
    AllocArray2D() = default;
    explicit AllocArray2D(std::size_t width, std::size_t height)
    {
        this->width = width;
        this->height = height;

        flattenArray2DVH = std::vector<T>(width * height);
    }

    const T& At(std::size_t y, std::size_t x) const
    {
        return flatArray2DElemRef(y, x);
    }
    T& At(std::size_t y, std::size_t x)
    {
        return flatArray2DElemRef(y, x);
    }

    std::size_t Width() const noexcept
    {
        return width;
    }
    std::size_t Height() const noexcept
    {
        return height;
    }

private:
    std::size_t width = 0, height = 0;

    std::vector<T> flattenArray2DVH{};

    const T& flatArray2DElemRef(std::size_t y, std::size_t x) const
    {
        return flattenArray2DVH[width * y + x];
    }
    T& flatArray2DElemRef(std::size_t y, std::size_t x)
    {
        return flattenArray2DVH[width * y + x];
    }
};
