#pragma once

#include <string.h>
#include <stdint.h>

#include <string>
#include <map>
#include <list>
#include <iomanip>
#include <iostream>
#include <sstream>

typedef void (*BestCallBack)(void* userData, const char* sig_code, bool trade_tick, double last_price, double turnover,
                             double deal_quantity, double bid_premium, double bid_demand, double ask_premium,
                             double ask_demand);
typedef void (*DepthCallBack)(void* userData, const char* sig_code, double bid_premium_1, double bid_demand_1,
                              double bid_premium_2, double bid_demand_2, double bid_premium_3, double bid_demand_3,
                              double bid_premium_4, double bid_demand_4, double bid_premium_5, double bid_demand_5,
                              double ask_premium_1, double ask_demand_1, double ask_premium_2, double ask_demand_2,
                              double ask_premium_3, double ask_demand_3, double ask_premium_4, double ask_demand_4,
                              double ask_premium_5, double ask_demand_5);
typedef void (*BookCallBack)(void* userData, const char* sig_code, bool trade_tick, double last_price, double turnover,
                             double deal_quantity, double bid_premium_0, double bid_demand_0, double bid_premium_1,
                             double bid_demand_1, double bid_premium_2, double bid_demand_2, double bid_premium_3,
                             double bid_demand_3, double bid_premium_4, double bid_demand_4, double ask_premium_0,
                             double ask_demand_0, double ask_premium_1, double ask_demand_1, double ask_premium_2,
                             double ask_demand_2, double ask_premium_3, double ask_demand_3, double ask_premium_4,
                             double ask_demand_4);

template<typename OrderId>
class Order
{
public:
    Order()
        : m_internalOrder(false)
        , m_hiddenOrder(false)
        , m_stamp(Stamp())
        , m_orderId()
        , m_exchangeId()
        , m_symbol()
        , m_isBuy(false)
        , m_isIoc(false)
        , m_price(0.0)
        , m_quantity(0)
        , m_openQuantity(0)
        , m_executedQuantity(0)
        , m_avgExecutedPrice(0.0)
        , m_lastExecutedPrice(0.0)
        , m_lastExecutedQuantity(0)
        , m_lastTradeStamp(0) {}

    Order(bool internalOrder, bool hiddenOrder, const OrderId& orderId, const std::string& clientId, const std::string& exchangeId,
          const std::string& symbol, bool isBuy, bool isIoc, double price, uint64_t quantity)
        : m_internalOrder(internalOrder)
        , m_hiddenOrder(hiddenOrder)
        , m_stamp(Stamp())
        , m_orderId(orderId)
        , m_clientId(clientId)
        , m_exchangeId(exchangeId)
        , m_symbol(symbol)
        , m_isBuy(isBuy)
        , m_isIoc(isIoc)
        , m_price(price)
        , m_quantity(quantity)
        , m_openQuantity(m_quantity)
        , m_executedQuantity(0)
        , m_avgExecutedPrice(0.0)
        , m_lastExecutedPrice(0.0)
        , m_lastExecutedQuantity(0)
        , m_lastTradeStamp(0) {}

    bool getInternal() const { return m_internalOrder; }
    bool getHidden() const { return m_hiddenOrder; }
    uint64_t getStamp() const { return m_stamp; }
    const OrderId& getOrderId() const { return m_orderId; }
    const std::string& getClientID() const { return m_clientId; }
    const std::string& getExchangeId() const { return m_exchangeId; }
    void setExchangeId(const std::string& exchangeId) { m_exchangeId = exchangeId; }
    const std::string& getSymbol() const { return m_symbol; }
    bool isBuy() const { return m_isBuy; }
    bool isIoc() const { return m_isIoc; }
    double getPrice() const { return m_price; }
    uint64_t getQuantity() const { return m_quantity; }
    void setQuantity(uint64_t quantity) { m_quantity = quantity; }
    uint64_t getOpenQuantity() const { return m_openQuantity; }
    void setOpenQuantity(uint64_t openQuantity) { m_openQuantity = openQuantity; }
    uint64_t getExecutedQuantity() const { return m_executedQuantity; }
    double getAvgExecutedPrice() const { return m_avgExecutedPrice; }
    double getLastExecutedPrice() const { return m_lastExecutedPrice; }
    uint64_t getLastExecutedQuantity() const { return m_lastExecutedQuantity; }
    uint64_t getLastExecutedSignedQuantity() const
    { return isBuy() ? m_lastExecutedQuantity : -m_lastExecutedQuantity; }
    uint64_t getLastTradeStamp() const { return m_lastTradeStamp; }
    std::string getLastTradeStampAsString() const
    {
        char buffer[16];
        sprintf(buffer, "%u", m_lastTradeStamp);
        return buffer;
    }

    bool isFilled() const { return m_quantity == m_executedQuantity; }
    bool isClosed() const { return m_openQuantity == 0; }

    void execute(double price, uint64_t quantity)
    {
        m_avgExecutedPrice =
            ((quantity * price) + (m_avgExecutedPrice * m_executedQuantity)) / (quantity + m_executedQuantity);
        m_openQuantity -= quantity;
        m_executedQuantity += quantity;
        m_lastExecutedPrice = price;
        m_lastExecutedQuantity = quantity;
        m_lastTradeStamp = Stamp();
    }

    void cancel() { m_openQuantity = 0; }

    Order AmendOrder(bool isPrxSet, double prx, bool isQtySet, uint64_t qty) const
    {
        Order amendedOrder(*this);

        amendedOrder.m_stamp = Stamp();
        if (isPrxSet)
            amendedOrder.m_price = prx;
        if (isQtySet)
        {
            int delta = (int)(qty - amendedOrder.m_quantity);
            int openQuantity = (amendedOrder.m_openQuantity + delta);
            amendedOrder.m_openQuantity = (openQuantity < 0) ? 0 : (uint64_t)openQuantity;
            amendedOrder.m_quantity = qty;
        }
        return amendedOrder;
    }

private:
    bool m_internalOrder;
    bool m_hiddenOrder;
    uint64_t m_stamp;
    OrderId m_orderId;
    std::string m_clientId;
    std::string m_exchangeId;
    std::string m_symbol;
    bool m_isBuy;
    bool m_isIoc;
    double m_price;
    uint64_t m_quantity;
    uint64_t m_openQuantity;
    uint64_t m_executedQuantity;
    double m_avgExecutedPrice;
    double m_lastExecutedPrice;
    uint64_t m_lastExecutedQuantity;
    uint64_t m_lastTradeStamp;
    static uint64_t Stamp()
    {
        static uint64_t stamp = 0;
        return ++stamp;
    }
};

template<typename OrderId>
inline std::ostream& operator<<(std::ostream& ostream, const Order<OrderId>& order)
{
    if (order.isBuy())
    {
        return ostream << std::setw(20) << order.getExchangeId() << std::setw(20) << order.getOpenQuantity()
                       << std::setw(20) << order.getPrice();
    }
    else
    {
        return ostream << std::setw(20) << order.getPrice() << std::setw(20) << order.getOpenQuantity() << std::setw(20)
                       << order.getExchangeId();
    }
}

template<typename OrderId>
class OrderBook
{
public:
    OrderBook(std::string symbol, BestCallBack bestCB = 0, DepthCallBack depthCB = 0, BookCallBack bookCB = 0,
              void* userData = 0)
        : m_symbol(symbol)
        , m_last_price(0.0)
        , m_turnover(0.0)
        , m_deal_quantity(0.0)
        , m_bestCB(bestCB)
        , m_depthCB(depthCB)
        , m_bookCB(bookCB)
        , m_userData(userData) {}

    const Order<OrderId>& insert(const Order<OrderId>& order)
    {
        if (order.isBuy())
            return m_bidOrders.emplace(order.getPrice(), order)->second;
        else
            return m_askOrders.emplace(order.getPrice(), order)->second;
    }
    void erase(const Order<OrderId>& order)
    {
        auto id = order.getExchangeId();
        if (order.isBuy())
        {
            for (auto i = m_bidOrders.begin(); i != m_bidOrders.end(); ++i)
                if (i->second.getExchangeId() == id)
                {
                    m_bidOrders.erase(i);
                    return;
                }
        }
        else
        {
            for (auto i = m_askOrders.begin(); i != m_askOrders.end(); ++i)
                if (i->second.getExchangeId() == id)
                {
                    m_askOrders.erase(i);
                    return;
                }
        }
    }
    Order<OrderId>& find(bool isBuy, std::string id)
    {
        if (isBuy)
        {
            for (auto i = m_bidOrders.begin(); i != m_bidOrders.end(); ++i)
                if (i->second.getExchangeId() == id)
                    return i->second;
        }
        else
        {
            for (auto i = m_askOrders.begin(); i != m_askOrders.end(); ++i)
                if (i->second.getExchangeId() == id)
                    return i->second;
        }
        throw std::exception();
    }
    uint64_t match(double* matchPrice = nullptr)
    {
        uint64_t tradeQty = 0;

        BidOrders bidOrders = m_bidOrders;
        AskOrders askOrders = m_askOrders;

        while (true)
        {
            if (bidOrders.empty() || askOrders.empty())
                return tradeQty;

            auto iBid = bidOrders.begin();
            auto iAsk = askOrders.begin();

            if (iBid->second.getPrice() >= iAsk->second.getPrice())
            {
                auto& bid = iBid->second;
                auto& ask = iAsk->second;

                match(bid, ask);
                tradeQty += bid.getLastExecutedQuantity();
                if (matchPrice)
                    (*matchPrice) = bid.getLastExecutedPrice();

                if (bid.isClosed())
                    bidOrders.erase(iBid);
                if (ask.isClosed())
                    askOrders.erase(iAsk);
            }
            else
                break;
        }

        return tradeQty;
    }
    bool match(std::list<Order<OrderId>>& orders, uint64_t maxTradeQty = 0)
    {
        bool modif = false;
        uint64_t turnover = 0;

        while (true)
        {
            if (m_bidOrders.empty() || m_askOrders.empty())
                break;

            auto iBid = m_bidOrders.begin();
            auto iAsk = m_askOrders.begin();

            if (iBid->second.getPrice() >= iAsk->second.getPrice())
            {
                modif = true;

                auto& bid = iBid->second;
                auto& ask = iAsk->second;

                match(bid, ask, maxTradeQty);
                orders.push_back(bid);
                orders.push_back(ask);

                m_last_price = bid.getLastExecutedPrice();
                m_deal_quantity = bid.getLastExecutedQuantity();
                m_turnover += m_deal_quantity;
                turnover += m_deal_quantity;

                if (bid.isClosed())
                    m_bidOrders.erase(iBid);
                if (ask.isClosed())
                    m_askOrders.erase(iAsk);

                publishTrade();

                if (maxTradeQty && (turnover >= maxTradeQty))
                    break;
            }
            else
                break;
        }

        if (modif)
            publishDepth();

        return (!orders.empty());
    }
    std::string display() const
    {
        std::stringstream stream;

        stream << "----------------------------------------------------------------------------------------------------"
                  "--------------------" << std::endl;
        stream << std::setw(20) << "Bid ExchId" << std::setw(20) << "Bid Quantity" << std::setw(20) << "Bid Price"
               << std::setw(20) << "Ask Price" << std::setw(20) << "Ask Quantity" << std::setw(20) << "Ask ExchId"
               << std::endl;
        stream << "----------------------------------------------------------------------------------------------------"
                  "--------------------" << std::endl;

        auto iBid = m_bidOrders.begin();
        auto iAsk = m_askOrders.begin();

        for (size_t idx = 0; (idx < m_bidOrders.size()) || (idx < m_askOrders.size()); ++idx)
        {
            if (iBid != m_bidOrders.end())
            {
                stream << iBid->second;
                ++iBid;
            }
            else
            {
                stream << std::setw(20) << "" << std::setw(20) << "" << std::setw(20) << "";
            }
            if (iAsk != m_askOrders.end())
            {
                stream << iAsk->second;
                ++iAsk;
            }
            else
            {
                stream << std::setw(20) << "" << std::setw(20) << "" << std::setw(20) << "";
            }

            stream << std::endl;
        }

        stream << "----------------------------------------------------------------------------------------------------"
                  "--------------------" << std::endl;

        return stream.str();
    }

    void getDepthForSide(bool isBuy, double* premium, double* demand, size_t count)
    {
        if (isBuy)
        {
            getDepth(m_bidOrders, premium, demand, count);
        }
        else
        {
            getDepth(m_askOrders, premium, demand, count);
        }
    }

    template <typename T>
    void getDepth(T orders, double* premium, double* demand, size_t count)
    {
        memset(premium, 0, sizeof(*premium) * count);
        memset(demand, 0, sizeof(*demand) * count);

        auto it = orders.begin();
        for (size_t idx = 0, pos = 0; (it != orders.end()) && (pos < count); ++idx, ++it)
        {
            if (idx == 0)
            {
                premium[pos] = it->second.getPrice();
                demand[pos] = it->second.getOpenQuantity();
            }
            else if (premium[pos] != it->second.getPrice())
            {
                ++pos;
                premium[pos] = it->second.getPrice();
                demand[pos] = it->second.getOpenQuantity();
            }
            else
            {
                demand[pos] += it->second.getOpenQuantity();
            }
        }
    }

    void publishTrade()
    {
        if (!m_bestCB && !m_bookCB)
            return;

        double bid_premium[5], bid_demand[5], ask_premium[5], ask_demand[5];
        getDepthForSide(true, bid_premium, bid_demand, 5);
        getDepthForSide(false, ask_premium, ask_demand, 5);

        if (m_bestCB)
            (*m_bestCB)(m_userData, m_symbol.c_str(), true, m_last_price, m_turnover, m_deal_quantity, bid_premium[0],
                        bid_demand[0], ask_premium[0], ask_demand[0]);
        if (m_bookCB)
            (*m_bookCB)(m_userData, m_symbol.c_str(), true, m_last_price, m_turnover, m_deal_quantity, bid_premium[0],
                        bid_demand[0], bid_premium[1], bid_demand[1], bid_premium[2], bid_demand[2], bid_premium[3],
                        bid_demand[3], bid_premium[4], bid_demand[4], ask_premium[0], ask_demand[0], ask_premium[1],
                        ask_demand[1], ask_premium[2], ask_demand[2], ask_premium[3], ask_demand[3], ask_premium[4],
                        ask_demand[4]);
    }

    void publishDepth()
    {
        if (!m_bestCB && !m_depthCB && !m_bookCB)
            return;

        double bid_premium[5], bid_demand[5], ask_premium[5], ask_demand[5];
        getDepthForSide(true, bid_premium, bid_demand, 5);
        getDepthForSide(false, ask_premium, ask_demand, 5);

        if (m_bestCB)
            (*m_bestCB)(m_userData, m_symbol.c_str(), false, m_last_price, m_turnover, m_deal_quantity, bid_premium[0],
                        bid_demand[0], ask_premium[0], ask_demand[0]);
        if (m_depthCB)
            (*m_depthCB)(m_userData, m_symbol.c_str(), bid_premium[0], bid_demand[0], bid_premium[1], bid_demand[1],
                         bid_premium[2], bid_demand[2], bid_premium[3], bid_demand[3], bid_premium[4], bid_demand[4],
                         ask_premium[0], ask_demand[0], ask_premium[1], ask_demand[1], ask_premium[2], ask_demand[2],
                         ask_premium[3], ask_demand[3], ask_premium[4], ask_demand[4]);
        if (m_bookCB)
            (*m_bookCB)(m_userData, m_symbol.c_str(), false, m_last_price, m_turnover, m_deal_quantity, bid_premium[0],
                        bid_demand[0], bid_premium[1], bid_demand[1], bid_premium[2], bid_demand[2], bid_premium[3],
                        bid_demand[3], bid_premium[4], bid_demand[4], ask_premium[0], ask_demand[0], ask_premium[1],
                        ask_demand[1], ask_premium[2], ask_demand[2], ask_premium[3], ask_demand[3], ask_premium[4],
                        ask_demand[4]);
    }

private:
    typedef std::multimap<double, Order<OrderId>, std::greater<double>> BidOrders;
    typedef std::multimap<double, Order<OrderId>, std::less<double>> AskOrders;

    void match(Order<OrderId>& bid, Order<OrderId>& ask, uint64_t maxTradeQty = 0)
    {
        double price = (bid.getStamp() > ask.getStamp()) ? ask.getPrice() : bid.getPrice();
        uint64_t quantity = 0;

        if (bid.getOpenQuantity() > ask.getOpenQuantity())
            quantity = ask.getOpenQuantity();
        else
            quantity = bid.getOpenQuantity();

        if (maxTradeQty && (quantity > (uint64_t)maxTradeQty))
            quantity = maxTradeQty;

        bid.execute(price, quantity);
        ask.execute(price, quantity);
    }

    std::string m_symbol;
    BidOrders m_bidOrders;
    AskOrders m_askOrders;
    double m_last_price;
    double m_turnover;
    double m_deal_quantity;
    BestCallBack m_bestCB;
    DepthCallBack m_depthCB;
    BookCallBack m_bookCB;
    void* m_userData;
};

template<typename OrderId>
class MatchingEngine
{
    typedef std::map<std::string, OrderBook<OrderId>> OrderBooks;
    typedef std::map<std::string, std::pair<std::string, bool>> OrderList;

public:
    MatchingEngine(BestCallBack bestCB = 0, DepthCallBack depthCB = 0, BookCallBack bookCB = 0, void* userData = 0)
        : m_bestCB(bestCB)
        , m_depthCB(depthCB)
        , m_bookCB(bookCB)
        , m_userData(userData) {}

    const Order<OrderId>& insert(const Order<OrderId>& order)
    {
        auto i = m_orderBooks.find(order.getSymbol());
        if (i == m_orderBooks.end())
            i = m_orderBooks.insert(std::make_pair(order.getSymbol(), OrderBook<OrderId>(order.getSymbol(), m_bestCB, m_depthCB,
                                                                                m_bookCB, m_userData))).first;

        const Order<OrderId>& newOrder = i->second.insert(order);
        i->second.publishDepth();
        m_orderList.insert(OrderList::value_type(order.getExchangeId(), make_pair(order.getSymbol(), order.isBuy())));
        return newOrder;
    }
    void erase(const Order<OrderId>& order)
    {
        auto i = m_orderBooks.find(order.getSymbol());
        if (i == m_orderBooks.end())
            return;
        m_orderList.erase(order.getExchangeId());
        i->second.erase(order);
        i->second.publishDepth();
    }
    Order<OrderId>& find(std::string id)
    {
        auto i = m_orderList.find(id);
        if (i == m_orderList.end())
            throw std::exception();
        return find(i->second.first, i->second.second, id);
    }
    Order<OrderId>& find(std::string symbol, bool isBuy, std::string id)
    {
        auto i = m_orderBooks.find(symbol);
        if (i == m_orderBooks.end())
            throw std::exception();
        return i->second.find(isBuy, id);
    }
    uint64_t match(std::string symbol, double* matchPrice = nullptr)
    {
        auto i = m_orderBooks.find(symbol);
        if (i == m_orderBooks.end())
            return false;
        return i->second.match(matchPrice);
    }
    bool match(std::string symbol, std::list<Order<OrderId>>& orders, uint64_t maxTradeQty = 0)
    {
        auto i = m_orderBooks.find(symbol);
        if (i == m_orderBooks.end())
            return false;
        return i->second.match(orders, maxTradeQty);
    }
    bool match(std::list<Order<OrderId>>& orders)
    {
        for (auto i = m_orderBooks.begin(); i != m_orderBooks.end(); ++i)
            i->second.match(orders);
        return !orders.empty();
    }
    std::string display(std::string symbol = std::string()) const
    {
        if (symbol.empty())
        {
            std::stringstream stream;
            for (auto it = m_orderBooks.begin(); it != m_orderBooks.end(); ++it)
            {
                stream << it->first << std::endl;
            }
            return stream.str();
        }
        else
        {
            auto i = m_orderBooks.find(symbol);
            if (i == m_orderBooks.end())
                return std::string();
            return i->second.display();
        }
    }

private:
    OrderBooks m_orderBooks;
    OrderList m_orderList;
    BestCallBack m_bestCB;
    DepthCallBack m_depthCB;
    BookCallBack m_bookCB;
    void* m_userData;
};
