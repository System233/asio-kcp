// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#ifndef ASIO_CONNECTION_MANAGER_H
#define ASIO_CONNECTION_MANAGER_H
#include <unordered_map>
#include <memory>
#include <boost/asio.hpp>

namespace std{
    using boost::asio::ip::udp;
    using connection_manager_hash_key_t=std::pair<udp::endpoint, intptr_t>;
    template<>
    struct hash<connection_manager_hash_key_t>{
        size_t operator()(connection_manager_hash_key_t const&key)const noexcept{
            return hash<udp::endpoint>()(key.first)^key.second;
        }
    };
}

namespace iudp
{
    using boost::asio::ip::udp;

    template <class protocol>
    class connection_channel;
    template <class protocol>
    class connection;

    template <class protocol>
    class connection_manager
    {

    public:
        using connection_channel_t=connection_channel<protocol>;
        using connection_manager_t=connection_manager<protocol>;
        using connection_t=connection<protocol>;

        connection_manager(connection_channel_t *ch) : m_channel(ch){};

        auto &connections() { return m_connections; }
        auto const&connections() const{ return m_connections; }
        auto begin() { return std::begin(connections()); }
        auto end() { return std::end(connections()); }
        auto insert(udp::endpoint const &endpoint, intptr_t id)
        {
            auto key = std::make_pair(endpoint, id);
            auto&conns = connections();
            return conns.insert(std::make_pair(key, std::make_unique<connection_t>(m_channel, endpoint, id)));
        }
        auto erase(udp::endpoint const &endpoint, intptr_t id)
        {
            auto key = std::make_pair(endpoint, id);
            auto&conns = connections();
            return conns.erase(key);
        }
        auto find(udp::endpoint const &endpoint, intptr_t id)
        {
            auto key = std::make_pair(endpoint, id);
            return connections().find(key);
        }

        connection_t *create_connection(udp::endpoint const &endpoint, intptr_t id)
        {
            return insert(endpoint, id).first->second.get();
        };
        bool remove_connection(udp::endpoint const &endpoint, intptr_t id)
        {
            return erase(endpoint, id) != 0;
        };
        bool remove_connection(connection_t*conn)
        {
            return erase(conn->endpoint(), conn->id());
        };
        connection_t *get_connection(udp::endpoint const &endpoint, intptr_t id)
        {
            auto it = find(endpoint, id);
            if (it != end())
            {
                return it->second.get();
            }
            return nullptr;
        };
        private:
            connection_channel_t *m_channel;
            std::unordered_map<std::pair<udp::endpoint, intptr_t>, std::unique_ptr<connection_t>> m_connections;

    };
}

#endif // ASIO_CONNECTION_MANAGER_H