package com.example.demo.service;

import java.math.BigDecimal;
import java.time.Instant;
import java.util.Comparator;
import java.util.PriorityQueue;

import org.springframework.stereotype.Service;

import com.example.demo.entity.PortfolioValueHistory;
import com.example.demo.entity.TradeType;
import com.example.demo.entity.Transaction;
import com.example.demo.repository.PortfolioRepository;
import com.example.demo.repository.PortfolioValueHistoryRepository;
import com.example.demo.repository.TransactionRepository;

@Service
public class OrderMatchingService {

    public static class Order {
        public enum Side { BUY, SELL }
        private final long portfolioId;
        private final String symbol;
        private final Side side;
        private int quantity;
        private final BigDecimal price;

        public Order(long portfolioId, String symbol, Side side, int quantity, BigDecimal price) {
            this.portfolioId = portfolioId;
            this.symbol = symbol;
            this.side = side;
            this.quantity = quantity;
            this.price = price;
        }
        // getters & setters…
        public long getPortfolioId() { return portfolioId; }
        public String getSymbol() { return symbol; }
        public Side getSide() { return side; }
        public int getQuantity() { return quantity; }
        public void setQuantity(int quantity) { this.quantity = quantity; }
        public BigDecimal getPrice() { return price; }
    }

    private final PriorityQueue<Order> buyBook =
        new PriorityQueue<>(Comparator.comparing(Order::getPrice).reversed());
    private final PriorityQueue<Order> sellBook =
        new PriorityQueue<>(Comparator.comparing(Order::getPrice));

    private final TransactionRepository txnRepo;
    private final PortfolioRepository portfolioRepo;
    private final MetricsService metricsService;
    private final PortfolioValueHistoryRepository valueHistoryRepo;

    public OrderMatchingService(TransactionRepository txnRepo,
                                PortfolioRepository portfolioRepo,
                                MetricsService metricsService,
                                PortfolioValueHistoryRepository valueHistoryRepo) {
        this.txnRepo = txnRepo;
        this.portfolioRepo = portfolioRepo;
        this.metricsService = metricsService;
        this.valueHistoryRepo = valueHistoryRepo;
    }

    public synchronized void placeOrder(Order order) {
        PriorityQueue<Order> opposite = (order.getSide() == Order.Side.BUY) ? sellBook : buyBook;
        while (order.getQuantity() > 0 && !opposite.isEmpty()) {
            Order best = opposite.peek();
            boolean match = (order.getSide() == Order.Side.BUY)
                ? best.getPrice().compareTo(order.getPrice()) <= 0
                : best.getPrice().compareTo(order.getPrice()) >= 0;
            if (!match) break;

            int qty = Math.min(order.getQuantity(), best.getQuantity());
            BigDecimal tradePrice = best.getPrice();
            executeTrade(order, best, qty, tradePrice);

            order.setQuantity(order.getQuantity() - qty);
            best.setQuantity(best.getQuantity() - qty);
            opposite.poll();
            if (best.getQuantity() > 0) opposite.add(best);
        }
        if (order.getQuantity() > 0) {
            if (order.getSide() == Order.Side.BUY) buyBook.add(order);
            else sellBook.add(order);
        }
        metricsService.publishOrderRate(1);
    }

    private void executeTrade(Order incoming, Order bookOrder, int qty, BigDecimal price) {
        // Persist one side of the trade (you could persist both if desired)
        Transaction t = new Transaction();
        var portfolio = portfolioRepo.findById(incoming.getPortfolioId()).orElseThrow();
        t.setPortfolio(portfolio);
        t.setStockSymbol(incoming.getSymbol());
        t.setQuantity(qty);
        t.setPrice(price);
        t.setType(incoming.getSide() == Order.Side.BUY ? TradeType.BUY : TradeType.SELL);
        txnRepo.save(t);
        // After saving the transaction, calculate and record portfolio value
        BigDecimal totalValue = calculatePortfolioValue(portfolio.getId());
        PortfolioValueHistory snapshot = new PortfolioValueHistory(portfolio, Instant.now(), totalValue);
        valueHistoryRepo.save(snapshot);
    }

    private BigDecimal calculatePortfolioValue(Long portfolioId) {
        // Simple sum of (shares * last price) for each stock in the portfolio
        var txns = txnRepo.findByPortfolioId(portfolioId);
        java.util.Map<String, Integer> holdings = new java.util.HashMap<>();
        java.util.Map<String, BigDecimal> lastPrice = new java.util.HashMap<>();
        for (var txn : txns) {
            String symbol = txn.getStockSymbol();
            int qty = txn.getType() == TradeType.BUY ? txn.getQuantity() : -txn.getQuantity();
            holdings.put(symbol, holdings.getOrDefault(symbol, 0) + qty);
            lastPrice.put(symbol, txn.getPrice());
        }
        BigDecimal total = BigDecimal.ZERO;
        for (var entry : holdings.entrySet()) {
            int shares = entry.getValue();
            if (shares > 0) {
                BigDecimal price = lastPrice.get(entry.getKey());
                total = total.add(price.multiply(BigDecimal.valueOf(shares)));
            }
        }
        return total;
    }
}
