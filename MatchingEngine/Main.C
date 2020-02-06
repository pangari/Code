#include "MatchingEngine.H"
#include "TimeNow.H"

#include <iostream>

MatchingEngine<std::string> engine_;

int main(int argc, char** argv)
{
    TimeNow now;
    MatchingEngine<TimeNow> engine;

    std::cout << engine.display() << std::endl;

    return (uint64_t)now;
}

/*
void onNew(MatchingEngine<TimeNow>& engine_)
{
    publishNew());

    if (!(qty % lotSize_) && (aty <= maxQty_))
    {
        Order<> order;
        {
            engine_.insert(order);
            acceptNewOrder(exchangeId);

            std::list<Order<OrderId>> orders;
            engine_.match(order.getSymbol(), orders);
            for (auto& match : orders)
            {
                double tradeTime = 0;
                executeOrder(match.getLastTradeStampAsString(),
                                match.getLastExecutedPrice(), match.getLastExecutedQuantity(),
                                match.getAvgExecutedPrice(), match.getExecutedQuantity(), tradeTime);
            }

            if (order.isClosed() == false)
            {
                if (order.isIoc())
                {
                    engine_.erase(order);
                    cancelAckOrder("IOC cancelled");
                }
            }
        }

        std::string output = engine_.display(order.getSymbol());
    }
    else
    {
        std::string errorText = (msg.Quantity() > maxQty_)
                                    ? "Invalid order size (> maximum size of " +
                                            ExpressTransport::StringUtil::ToString(maxQty_) + ")"
                                    : (msg.Quantity() < lotSize_)
                                            ? "Invalid order size (< minimum size of " +
                                                std::to_string(lotSize_) + ")"
                                            : "Invalid order size (not multiple of lot size " +
                                                std::to_string(lotSize_) + ")";
        rejectCommand(errorText);
    }
}
void onAmend(MatchingEngine<TimeNow>& engine_)
{
    publishAmend();

    try
    {
        Order<OrderId>& order = engine_.find(orderId);
        Order<OrderId> amendedOrder = order.AmendOrder(isPrxSet, prx, isQtySet, qty);

        engine_.erase(order);
        if (amendedOrder.isClosed())
        {
            cancelAckOrder("Amended quantity to zero");
        }
        else
        {
            amendAckOrder();
            engine_.insert(amendedOrder);

            std::list<Order<OrderId>> orders;
            engine_.match(amendedOrder.getSymbol(), orders);
            for (auto& match : orders)
            {
                double tradeTime = 0;
                executeOrder(match.getLastTradeStampAsString()
                            match.getLastExecutedPrice(), match.getLastExecutedQuantity(),
                            match.getAvgExecutedPrice(), match.getExecutedQuantity(), tradeTime);
            }
        }

        std::string output = engine_.display(amendedOrder.getSymbol());
    }
    catch (...)
    {
        rejectAmend("Unable to find order");
    }
}
void onCancel(MatchingEngine<TimeNow>& engine_)
{
    publishCancel();

    try
    {
        Order<OrderId>& order = engine_.find(orderId);
        order.cancel();
        cancelAckOrder("Remaining quantity cancelled");

        std::string symbol = order.getSymbol();
        engine_.erase(order);

        std::string output = engine_.display(symbol);
    }
    catch (...)
    {
        rejectCommand("Unable to find order");
    }
}
*/