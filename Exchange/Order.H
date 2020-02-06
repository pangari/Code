#pragma once

#include <string.h>
#include <stdint.h>

#include <string>
#include <iomanip>
#include <iostream>

template<typename OrderKey>
class Order final
{
public:
    Order()
        : internalOrder_(false)
        , hiddenOrder_(false)
        , stamp_(Stamp())
        , orderKey_()
        , clientId_()
        , exchangeId_()
        , symbol_()
        , isBuy_(false)
        , isIoc_(false)
        , prx_(0.0)
        , qty_(0)
        , openQuantity_(0)
        , executedQuantity_(0)
        , avgExecutedPrice_(0.0)
        , lastExecutedPrice_(0.0)
        , lastExecutedQuantity_(0)
        , lastTradeStamp_(0) {}

    Order(const bool internalOrder, const bool hiddenOrder, const OrderKey& orderKey, const std::string& clientId, const std::string& exchangeId,
          const std::string& symbol, const bool isBuy, const bool isIoc, const double prx, const int64_t qty)
        : internalOrder_(internalOrder)
        , hiddenOrder_(hiddenOrder)
        , stamp_(Stamp())
        , orderKey_(orderKey)
        , clientId_(clientId)
        , exchangeId_(exchangeId)
        , symbol_(symbol)
        , isBuy_(isBuy)
        , isIoc_(isIoc)
        , prx_(prx)
        , qty_(qty)
        , openQuantity_(qty_)
        , executedQuantity_(0)
        , avgExecutedPrice_(0.0)
        , lastExecutedPrice_(0.0)
        , lastExecutedQuantity_(0)
        , lastTradeStamp_(0) {}

    bool getInternal() const { return internalOrder_; }
    bool getHidden() const { return hiddenOrder_; }
    int64_t getStamp() const { return stamp_; }
    const OrderKey& getOrderKey() const { return orderKey_; }
    const std::string& getClientID() const { return clientId_; }
    const std::string& getExchangeId() const { return exchangeId_; }
    void setExchangeId(const std::string& exchangeId) { exchangeId_ = exchangeId; }
    const std::string& getSymbol() const { return symbol_; }
    bool isBuy() const { return isBuy_; }
    bool isIoc() const { return isIoc_; }
    double getPrice() const { return prx_; }
    int64_t getQuantity() const { return qty_; }
    void setQuantity(int64_t qty) { qty_ = qty; }
    int64_t getOpenQuantity() const { return openQuantity_; }
    void setOpenQuantity(const int64_t openQuantity) { openQuantity_ = openQuantity; }
    int64_t getExecutedQuantity() const { return executedQuantity_; }
    double getAvgExecutedPrice() const { return avgExecutedPrice_; }
    double getLastExecutedPrice() const { return lastExecutedPrice_; }
    int64_t getLastExecutedQuantity() const { return lastExecutedQuantity_; }
    int64_t getLastExecutedSignedQuantity() const
    { return isBuy() ? lastExecutedQuantity_ : -lastExecutedQuantity_; }
    int64_t getLastTradeStamp() const { return lastTradeStamp_; }
    std::string getLastTradeStampAsString() const { return std::to_string(lastTradeStamp_); }

    bool isFilled() const { return qty_ == executedQuantity_; }
    bool isClosed() const { return openQuantity_ == 0; }

    void execute(const double prx, const int64_t qty)
    {
        avgExecutedPrice_ =
            ((qty * prx) + (avgExecutedPrice_ * executedQuantity_)) / (qty + executedQuantity_);
        openQuantity_ -= qty;
        executedQuantity_ += qty;
        lastExecutedPrice_ = prx;
        lastExecutedQuantity_ = qty;
        lastTradeStamp_ = Stamp();
    }

    void cancel() { openQuantity_ = 0; }

    Order AmendOrder(const bool isPrxSet, const double prx, const bool isQtySet, const int64_t qty) const
    {
        Order amendedOrder(*this);

        amendedOrder.stamp_ = Stamp();
        if (isPrxSet)
            amendedOrder.prx_ = prx;
        if (isQtySet)
        {
            int64_t delta = (qty - amendedOrder.qty_);
            int64_t openQuantity = (amendedOrder.openQuantity_ + delta);
            amendedOrder.openQuantity_ = (openQuantity < 0) ? 0 : openQuantity;
            amendedOrder.qty_ = qty;
        }
        return amendedOrder;
    }

private:
    bool internalOrder_;
    bool hiddenOrder_;
    int64_t stamp_;
    OrderKey orderKey_;
    std::string clientId_;
    std::string exchangeId_;
    std::string symbol_;
    bool isBuy_;
    bool isIoc_;
    double prx_;
    int64_t qty_;
    int64_t openQuantity_;
    int64_t executedQuantity_;
    double avgExecutedPrice_;
    double lastExecutedPrice_;
    int64_t lastExecutedQuantity_;
    int64_t lastTradeStamp_;
    static int64_t Stamp()
    {
        static int64_t stamp = 0;
        return ++stamp;
    }
};

template<typename OrderKey>
inline std::ostream& operator<<(std::ostream& ostream, const Order<OrderKey>& order)
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
