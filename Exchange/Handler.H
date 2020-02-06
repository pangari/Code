#include "Engine.H"

template<typename OrderKey>
class Handler
{
public:
    Handler(int64_t maxQty = 1000, int64_t lotSize = 1) 
            : engine_(*this)  
            , maxQty_(maxQty)
            , lotSize_(lotSize) {}

    template<typename Functor>
    void setOnBestData(Functor&& onBestData) { onBestData_ = std::move(onBestData); }
    OnBestData& getOnBestData() { return onBestData_; }

    template<typename Functor>
    void setOnDepthData(Functor&& onDepthData) { onDepthData_ = std::move(onDepthData); }
    OnDepthData& getOnDepthData() { return onDepthData_; }

    std::string procesNew(const OrderKey& orderKey, const std::string& symbol, const bool isBuy, const bool isIoc, const double prx, const int64_t qty)
    {
        std::string id = std::to_string(orderKey);

        //publishNew(orderKey, prxm qty);

        if (!(qty % lotSize_) && (qty <= maxQty_))
        {
            Order<OrderKey> order(false, false, orderKey, id, id, symbol, isBuy, isIoc, prx, qty);
            {
                engine_.insert(order);
                //acceptNewOrder(exchangeId);

                std::list<Order<OrderKey>> orders;
                engine_.match(order.getSymbol(), orders);
                /*
                for (auto& match : orders)
                {
                    double tradeTime = 0;
                    publishExecute(match.getLastTradeStampAsString(),
                                    match.getLastExecutedPrice(), match.getLastExecutedQuantity(),
                                    match.getAvgExecutedPrice(), match.getExecutedQuantity(), tradeTime);
                }
                */

                if (order.isClosed() == false)
                {
                    if (order.isIoc())
                    {
                        engine_.erase(order);
                        //cancelAckOrder(orderKey, "IOC cancelled");
                    }
                }
            }
        }
        else
        {
            std::string errorText = (qty > maxQty_)
                                        ? "Invalid order size (> maximum size of " + std::to_string(maxQty_) + ")"
                                        : (qty < lotSize_)
                                                ? "Invalid order size (< minimum size of " + std::to_string(lotSize_) + ")"
                                                : "Invalid order size (not multiple of lot size " + std::to_string(lotSize_) + ")";
            //rejectCommand(orderKey, errorText);
        }

        return id;
    }

    void procesAmend(const std::string& id, const double *optionalPrx = nullptr, const int64_t *optionalQty = nullptr)
    {
        if(!optionalPrx && !optionalQty)
            return;

        //publishAmend(orderKey, prx, qty);

        double prx = (optionalPrx ? *optionalPrx : 0.0);
        int64_t qty = (optionalQty ? *optionalQty : 0.0);

        try
        {
            Order<OrderKey>& order = engine_.find(id);
            Order<OrderKey> amendedOrder = order.AmendOrder(optionalPrx, prx, optionalQty, qty);

            engine_.erase(order);
            if (amendedOrder.isClosed())
            {
                //cancelAckOrder(orderKey, "Amended quantity to zero");
            }
            else
            {
                //amendAckOrder(orderKey, prx, qty);
                engine_.insert(amendedOrder);

                std::list<Order<OrderKey>> orders;
                engine_.match(amendedOrder.getSymbol(), orders);
                /*
                for (auto& match : orders)
                {
                    double tradeTime = 0;
                    publishExecute(match.getLastTradeStampAsString()
                                match.getLastExecutedPrice(), match.getLastExecutedQuantity(),
                                match.getAvgExecutedPrice(), match.getExecutedQuantity(), tradeTime);
                }
                */
            }

            std::string output = engine_.display(amendedOrder.getSymbol());
        }
        catch (...)
        {
            //rejectAmend(orderKey, "Unable to find order");
        }
    }

    void procesCancel(const std::string& id)
    {
        //publishCancel(orderKey);

        try
        {
            Order<OrderKey>& order = engine_.find(id);
            order.cancel();
            //cancelAckOrder("Remaining quantity cancelled");

            std::string symbol = order.getSymbol();
            engine_.erase(order);
        }
        catch (...)
        {
            //rejectCommand(orderKey, "Unable to find order");
        }
    }

    std::string display(std::string symbol = std::string()) const
    {
        return engine_.display(symbol);
    }

private:
    Engine<Handler, OrderKey> engine_;
    int64_t maxQty_;
    int64_t lotSize_;
    OnBestData onBestData_;
    OnDepthData onDepthData_;
};
