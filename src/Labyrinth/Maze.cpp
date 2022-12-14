#include "Maze.h"

#include <algorithm>
#include <array>

Maze::Maze(size_t width, size_t height, uint32_t seed)
    : m_grid(width, height)
    , m_randGenerator(seed)
{
    CreateMaze();
}

Maze::Maze(size_t width, size_t height)
    : m_grid(width, height)
{
    std::seed_seq sseq = {width, height};
    m_randGenerator = std::mt19937(sseq);
    CreateMaze();
}

void Maze::CreateMaze()
{
    // pair (x, y) - position in the grid
    std::stack<Vec2sz> mazeStack;

    mazeStack.push( {0, 0} );
    m_grid[0][0].setVisited();

    // we will store all available directions here
    std::vector<Vec2i> neighbors;
    neighbors.reserve(4); // avoid reallocations

    // we could utilize an array of neighbors to reduce shitty code
    static constexpr std::array<Vec2i, 4> directions = {
        Vec2i( 0, -1),
        Vec2i( 1,  0),
        Vec2i( 0,  1),
        Vec2i(-1,  0),
    };

    while (!mazeStack.empty())
    {
        Vec2sz pos = mazeStack.top();

        for (Vec2i delta : directions)
        {
            // Be careful here with conversions as we try to add unsigned type to signed (int32_t and size_t)
            Vec2sz npos = Vec2sz(pos.x + delta.x, pos.y + delta.y);
            if (npos.x > m_grid.getWidth() - 1 || npos.y > m_grid.getHeight() - 1 || m_grid[npos.x][npos.y].isVisited())
                continue;

            neighbors.push_back(delta);
        }

        if (!neighbors.empty())
        {
            // get location of random neighbor
            Vec2i delta = neighbors.at(generateIndex(0, neighbors.size() - 1));
            // we won't use operator+ overload here as we treat two different types
            // as you may have seen above
            Vec2sz npos = Vec2sz(pos.x + delta.x, pos.y + delta.y);

            // get neighboring cell
            Cell& ncell = m_grid[npos.x][npos.y];
            // get parent cell
            Cell& pcell = m_grid[pos.x][pos.y];

            // we break a wall in both cells and mark them as visited
            pcell.setVisited();
            ncell.setVisited();
            pcell.breakWall((Direction) delta);
            ncell.breakWall(getOpposite((Direction) delta));
            mazeStack.push(npos);

            // capacity of vector is untouched, so no std::vector::reserve() is needed
            neighbors.clear();
        }
        else mazeStack.pop();
    }
}

void Maze::Print(std::optional<Maze::cref_type<Maze::path_container_type>> path)
{
    bool pathWay = false;

    auto is_path = [&](size_t x, size_t y) -> bool
    {
        if (!path.has_value()) // so that the printing part is cleaner
            return false;

        const Maze::path_container_type& container = path.value().get();
        // be careful with size_t -> int32_t conversion
        Vec2sz target = Vec2sz(x, y); // Vector, that we want to find in path
        return std::any_of(container.begin(), container.end(), [&](const Vec2sz& v) { return v == target; });
    };

    for (size_t y = 0; y < m_grid.getHeight(); y++)
    {
        for (size_t x = 0; x < m_grid.getWidth(); x++)
        {
            pathWay = is_path(x, y);

            if (pathWay)
            {
                std::cout << (!m_grid[x][y].hasPath(Direction::NORTH) ? "##" : "#.");
            }
            else
            {
                std::cout << (!m_grid[x][y].hasPath(Direction::NORTH) ? "##" : "# ");
            }

            pathWay = false;
        }
        std::cout << "#\n";

        for (size_t x = 0; x < m_grid.getWidth(); x++)
        {
            pathWay = is_path(x, y);

            if (pathWay)
            {
                std::cout << (!m_grid[x][y].hasPath(Direction::WEST) ? "#." : "..");
            }
            else
            {
                std::cout << (!m_grid[x][y].hasPath(Direction::WEST) ? "# " : "  ");
            }

            pathWay = false;
        }
        std::cout << "#\n";
    }

    for (size_t x = 0; x < m_grid.getWidth(); x++)
    {
        std::cout << "##";
    }

    std::cout << "#\n";
}

void Maze::PrintBold(std::optional<Maze::cref_type<Maze::path_container_type>> path)
{
    // k = 0 -	###	\
	// k = 1 -	# #	- entire cell spans 3 rows
    // k = 2 -	###	/

    bool pathWay = false;

    auto is_path = [&](size_t x, size_t y) -> bool
    {
        if (!path.has_value()) // so that the printing part is cleaner
            return false;

        const Maze::path_container_type& container = path.value().get();
        // be careful with size_t -> int32_t conversion
        Vec2sz target = Vec2sz(x, y); // Vector, that we want to find in path
        return std::any_of(container.begin(), container.end(), [&](const Vec2sz& v) { return v == target; });
    };

    for (size_t y = 0; y < m_grid.getHeight(); y++)
    {
        for (size_t k = 0; k < 3; k++)
        {
            std::cout << "#";

            for (size_t x = 0; x < m_grid.getWidth(); x++)
            {
                Cell& curr = m_grid[x][y];

                pathWay = is_path(x, y);

                switch (k)
                {
                case 0:
                    if (!curr.hasPath(Direction::NORTH))
                    {
                        std::cout << "###";
                    }
                    else
                    {
                        std::cout << (pathWay ? "#.#" : "# #");
                    }
                    break;
                case 1:
                    if (!curr.hasPath(Direction::EAST) && curr.hasPath(Direction::WEST))
                    {
                        std::cout << (pathWay ? "..#" : "  #");
                    }
                    else if (curr.hasPath(Direction::EAST) && !curr.hasPath(Direction::WEST))
                    {
                        std::cout << (pathWay ? "#.." : "#  ");
                    }
                    else if (!curr.hasPath(Direction::EAST) && !curr.hasPath(Direction::WEST))
                    {
                        std::cout << (pathWay ? "#.#" : "# #");
                    }
                    else
                    {
                        std::cout << (pathWay ? "..." : "   ");
                    }
                    break;
                case 2:
                    if (!curr.hasPath(Direction::SOUTH))
                    {
                        std::cout << "###";
                    }
                    else
                    {
                        std::cout << (pathWay ? "#.#" : "# #");
                    }
                    break;
                default:
                    break;
                }

                pathWay = false;
            }

            std::cout << "#\n";
        }
    }
}

size_t Maze::generateIndex(size_t lhs, size_t rhs)
{
    std::uniform_int_distribution<size_t> randDist(lhs, rhs);
    return randDist(m_randGenerator);
}