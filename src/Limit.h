#pragma once

#include <utility>

struct Quote
{
    double Price { 0 };
    double Quantity { 0 };

    Quote() : Price(0), Quantity(0) {}
    Quote(double price, double qty) : Price(price), Quantity(qty) {}
};

struct Limit
{
    Quote Bid;
    Quote Ask;

    Limit(): Bid(), Ask() {}
    Limit(double bidPrice, double bidQty, double askPrice, double askQty) : Bid(bidPrice, bidQty), Ask(askPrice, askQty) {}
};
