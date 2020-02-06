#pragma once

#include "Order.H"

#include <string.h>
#include <stdint.h>

#include <string>
#include <map>
#include <list>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <functional>

typedef double (&PrxArr)[5];
typedef double (&PrxArr)[5];
typedef int64_t (&QtyArr)[5];
typedef int64_t (&QtyArr)[5];

typedef std::function<void(const std::string& symbol, const bool tradeTick, const double lastPrice, const int64_t tradeQuantity, 
                           const int64_t turnover, const double bidPrx, const int64_t bidQty, const double askPrx,
                           const int64_t askQty)> OnBestData;

typedef std::function<void(const std::string& symbol, const bool tradeTick, const double lastPrice, const int64_t tradeQuantity, 
                           const int64_t turnover, const PrxArr& bidPrx, const QtyArr& bidQty, 
                           const PrxArr& askPrx, const QtyArr& askQty)> OnDepthData;

template<typename Handler, typename OrderKey>
class OrderBook final
{
public:
    OrderBook(Handler& handler, std::string symbol)
        : handler_(handler)
        , symbol_(symbol)
        , lastPrice_(0.0)
        , dealQuantity_(0.0)
        , turnover_(0.0) {}

    const Order<OrderKey>& insert(const Order<OrderKey>& order)
    {
        if (order.isBuy())
            return bidOrders_.emplace(order.getPrice(), order)->second;
        else
            return askOrders_.emplace(order.getPrice(), order)->second;
    }
    void erase(const Order<OrderKey>& order)
    {
        auto id = order.getExchangeId();
        if (order.isBuy())
        {
            for (auto i = bidOrders_.begin(); i != bidOrders_.end(); ++i)
                if (i->second.getExchangeId() == id)
                {
                    bidOrders_.erase(i);
                    return;
                }
        }
        else
        {
            for (auto i = askOrders_.begin(); i != askOrders_.end(); ++i)
                if (i->second.getExchangeId() == id)
                {
                    askOrders_.erase(i);
                    return;
                }
        }
    }
    Order<OrderKey>& find(const bool isBuy, const std::string& id)
    {
        if (isBuy)
        {
            for (auto i = bidOrders_.begin(); i != bidOrders_.end(); ++i)
                if (i->second.getExchangeId() == id)
                    return i->second;
        }
        else
        {
            for (auto i = askOrders_.begin(); i != askOrders_.end(); ++i)
                if (i->second.getExchangeId() == id)
                    return i->second;
        }
        throw std::exception();
    }
    int64_t match(double* matchPrice = nullptr)
    {
        int64_t tradeQty = 0;

        BidOrders bidOrders = bidOrders_;
        AskOrders askOrders = askOrders_;

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
    bool match(std::list<Order<OrderKey>>& orders, const int64_t maxTradeQty = 0)
    {
        bool modif = false;
        int64_t turnover = 0;

        while (true)
        {
            if (bidOrders_.empty() || askOrders_.empty())
                break;

            auto iBid = bidOrders_.begin();
            auto iAsk = askOrders_.begin();

            if (iBid->second.getPrice() >= iAsk->second.getPrice())
            {
                modif = true;

                auto& bid = iBid->second;
                auto& ask = iAsk->second;

                match(bid, ask, maxTradeQty);
                orders.push_back(bid);
                orders.push_back(ask);

                lastPrice_ = bid.getLastExecutedPrice();
                dealQuantity_ = bid.getLastExecutedQuantity();
                turnover_ += dealQuantity_;
                turnover += dealQuantity_;

                if (bid.isClosed())
                    bidOrders_.erase(iBid);
                if (ask.isClosed())
                    askOrders_.erase(iAsk);

                publishMarketData(true);

                if (maxTradeQty && (turnover >= maxTradeQty))
                    break;
            }
            else
                break;
        }

        if (modif)
            publishMarketData();

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

        auto iBid = bidOrders_.begin();
        auto iAsk = askOrders_.begin();

        for (size_t idx = 0; (idx < bidOrders_.size()) || (idx < askOrders_.size()); ++idx)
        {
            if (iBid != bidOrders_.end())
            {
                stream << iBid->second;
                ++iBid;
            }
            else
            {
                stream << std::setw(20) << "" << std::setw(20) << "" << std::setw(20) << "";
            }
            if (iAsk != askOrders_.end())
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

    template<size_t N>
    void getDepthForSide(bool isBuy, double (&prx)[N], int64_t (&qty)[N])
    {
        if (isBuy)
        {
            getDepth(bidOrders_, prx, qty);
        }
        else
        {
            getDepth(askOrders_, prx, qty);
        }
    }

    template <typename T, size_t N>
    void getDepth(T orders, double (&prx)[N], int64_t (&qty)[N])
    {
        memset(&prx, 0, sizeof(prx));
        memset(&qty, 0, sizeof(qty));

        auto it = orders.begin();
        for (size_t idx = 0, pos = 0; (it != orders.end()) && (pos < N); ++idx, ++it)
        {
            if (idx == 0)
            {
                prx[pos] = it->second.getPrice();
                qty[pos] = it->second.getOpenQuantity();
            }
            else if (prx[pos] != it->second.getPrice())
            {
                ++pos;
                prx[pos] = it->second.getPrice();
                qty[pos] = it->second.getOpenQuantity();
            }
            else
            {
                qty[pos] += it->second.getOpenQuantity();
            }
        }
    }

    void publishMarketData(const bool isTrade = false)
    {
        OnBestData& onBestData = handler_.getOnBestData();
        OnDepthData& onDepthData = handler_.getOnDepthData();

        if (!onBestData && !onDepthData)
            return;

        double bidPrx[5], askPrx[5];
        int64_t bidQty[5], askQty[5];

        getDepthForSide(true, bidPrx, bidQty);
        getDepthForSide(false, askPrx, askQty);

        if (onBestData)
            onBestData(symbol_, isTrade, lastPrice_, dealQuantity_, turnover_,
                        bidPrx[0], bidQty[0], askPrx[0], askQty[0]);

        if (onDepthData)
            onDepthData(symbol_, isTrade, lastPrice_, dealQuantity_, turnover_, bidPrx, bidQty, askPrx, askQty);
    }

private:
    typedef std::multimap<double, Order<OrderKey>, std::greater<double>> BidOrders;
    typedef std::multimap<double, Order<OrderKey>, std::less<double>> AskOrders;

    void match(Order<OrderKey>& bid, Order<OrderKey>& ask, const int64_t maxTradeQty = 0)
    {
        double prx = (bid.getStamp() > ask.getStamp()) ? ask.getPrice() : bid.getPrice();
        int64_t qty = 0;

        if (bid.getOpenQuantity() > ask.getOpenQuantity())
            qty = ask.getOpenQuantity();
        else
            qty = bid.getOpenQuantity();

        if (maxTradeQty && (qty > maxTradeQty))
            qty = maxTradeQty;

        bid.execute(prx, qty);
        ask.execute(prx, qty);
    }

    Handler& handler_;
    std::string symbol_;
    BidOrders bidOrders_;
    AskOrders askOrders_;
    double lastPrice_;
    int64_t dealQuantity_;
    int64_t turnover_;
};
