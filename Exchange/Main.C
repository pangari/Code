#include "Handler.H"
#include "TimeNow.H"

#include <iostream>

void PrintBestData(const std::string& symbol, const bool tradeTick, const double lastPrice, const int64_t tradeQuantity,
                   const int64_t turnover, const double bidPrx, const int64_t bidQty, const double askPrx,
                   const int64_t askQty)
{
    fprintf(stdout, "Sym<%s>, Tick<%s>, Last<%ld@%f>[%ld] Bid<%ld@%f> Ask<%ld@%f>\n", symbol.c_str(), (tradeTick ? "true" : "false"), 
            tradeQuantity, lastPrice, turnover, 
            bidQty, bidPrx, askQty, askPrx);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    TimeNow now;
    Handler<TimeNow> handler;

    handler.setOnBestData(PrintBestData);

    std::string sym = "BHP.AX";
    double prx = 123.456;
    int64_t qty = 100;
    std::string id;

    id = handler.procesNew(now, sym, true, false, prx, qty);
    //std::cout << handler.display(sym) << std::endl;

    prx = 456.789;
    id = handler.procesNew(now, sym, false, false, prx, 10);
    id = handler.procesNew(now, sym, false, false, prx, 10);
    id = handler.procesNew(now, sym, false, false, prx, 10);
    id = handler.procesNew(now, sym, false, false, prx, 10);
    //std::cout << handler.display(sym) << std::endl;

    prx = 456.789;
    handler.procesAmend(id, &prx);
    //std::cout << handler.display(sym) << std::endl;

    handler.procesCancel(id);
    //std::cout << handler.display(sym) << std::endl;

    return (uint64_t)now;
}
