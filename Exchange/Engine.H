#pragma once

#include "Order.H"
#include "OrderBook.H"

#include <string.h>
#include <stdint.h>

#include <string>
#include <map>
#include <list>

template<typename Handler, typename OrderKey>
class Engine final
{
    typedef std::map<std::string, OrderBook<Handler, OrderKey>> OrderBooks;
    typedef std::map<std::string, std::pair<std::string, bool>> OrderList;

public:
    Engine(Handler& handler) : handler_(handler) {}

    const Order<OrderKey>& insert(const Order<OrderKey>& order)
    {
        auto i = orderBooks_.find(order.getSymbol());
        if (i == orderBooks_.end())
            i = orderBooks_.insert(std::make_pair(order.getSymbol(), OrderBook<Handler, OrderKey>(handler_, order.getSymbol()))).first;

        const Order<OrderKey>& newOrder = i->second.insert(order);
        i->second.publishMarketData();
        orderList_.insert(OrderList::value_type(order.getExchangeId(), make_pair(order.getSymbol(), order.isBuy())));
        return newOrder;
    }
    void erase(const Order<OrderKey>& order)
    {
        auto i = orderBooks_.find(order.getSymbol());
        if (i == orderBooks_.end())
            return;
        orderList_.erase(order.getExchangeId());
        i->second.erase(order);
        i->second.publishMarketData();
    }
    Order<OrderKey>& find(const std::string& id)
    {
        auto i = orderList_.find(id);
        if (i == orderList_.end())
            throw std::exception();
        return find(i->second.first, i->second.second, id);
    }
    Order<OrderKey>& find(const std::string& symbol, const bool isBuy, const std::string& id)
    {
        auto i = orderBooks_.find(symbol);
        if (i == orderBooks_.end())
            throw std::exception();
        return i->second.find(isBuy, id);
    }
    int64_t match(const std::string& symbol, double* matchPrice = nullptr)
    {
        auto i = orderBooks_.find(symbol);
        if (i == orderBooks_.end())
            return false;
        return i->second.match(matchPrice);
    }
    bool match(const std::string& symbol, std::list<Order<OrderKey>>& orders, const int64_t maxTradeQty = 0)
    {
        auto i = orderBooks_.find(symbol);
        if (i == orderBooks_.end())
            return false;
        return i->second.match(orders, maxTradeQty);
    }
    bool match(std::list<Order<OrderKey>>& orders)
    {
        for (auto i = orderBooks_.begin(); i != orderBooks_.end(); ++i)
            i->second.match(orders);
        return !orders.empty();
    }
    std::string display(std::string symbol = std::string()) const
    {
        if (symbol.empty())
        {
            std::stringstream stream;
            for (auto it = orderBooks_.begin(); it != orderBooks_.end(); ++it)
            {
                stream << it->first << std::endl;
            }
            return stream.str();
        }
        else
        {
            auto i = orderBooks_.find(symbol);
            if (i == orderBooks_.end())
                return std::string();
            return i->second.display();
        }
    }

private:
    Handler& handler_;
    OrderBooks orderBooks_;
    OrderList orderList_;
};
