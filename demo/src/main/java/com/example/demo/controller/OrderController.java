package com.example.demo.controller;

import java.math.BigDecimal;
import java.time.Instant;

import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.example.demo.entity.Portfolio;
import com.example.demo.entity.PortfolioValueHistory;
import com.example.demo.entity.Stock;
import com.example.demo.entity.TradeType;
import com.example.demo.entity.Transaction;
import com.example.demo.repository.PortfolioRepository;
import com.example.demo.repository.PortfolioValueHistoryRepository;
import com.example.demo.repository.StockRepository;
import com.example.demo.repository.TransactionRepository;

@RestController
@RequestMapping("/api/orders")
@Component
public class OrderController {
    private final TransactionRepository transactionRepository;
    private final PortfolioRepository portfolioRepository;
    private final StockRepository stockRepository;
    private final PortfolioValueHistoryRepository valueHistoryRepository;

    public OrderController(TransactionRepository transactionRepository,
                         PortfolioRepository portfolioRepository,
                         StockRepository stockRepository,
                         PortfolioValueHistoryRepository valueHistoryRepository) {
        this.transactionRepository = transactionRepository;
        this.portfolioRepository = portfolioRepository;
        this.stockRepository = stockRepository;
        this.valueHistoryRepository = valueHistoryRepository;
    }

    @PostMapping
    public ResponseEntity<Transaction> placeOrder(@RequestBody OrderRequest request) {
        try {
            System.out.println("Received order request: " + request);
            
            // Find portfolio
            Portfolio portfolio = portfolioRepository.findById(request.getPortfolioId())
                .orElseThrow(() -> new RuntimeException("Portfolio not found"));
            System.out.println("Found portfolio: " + portfolio.getId());

            // Find or create stock
            Stock stock = stockRepository.findById(request.getSymbol())
                .orElseGet(() -> {
                    Stock newStock = new Stock(request.getSymbol(), request.getSymbol());
                    return stockRepository.save(newStock);
                });
            System.out.println("Found/created stock: " + stock.getSymbol());

            // Create transaction
            Transaction transaction = new Transaction();
            transaction.setPortfolio(portfolio);
            transaction.setStockSymbol(request.getSymbol());
            transaction.setQuantity(request.getQuantity());
            transaction.setPrice(BigDecimal.valueOf(request.getPrice()));
            transaction.setType(request.getSide().equals("BUY") ? TradeType.BUY : TradeType.SELL);

            System.out.println("Saving transaction...");
            Transaction savedTransaction = transactionRepository.save(transaction);
            System.out.println("Transaction saved with ID: " + savedTransaction.getId());
            
            try {
                // Calculate and save portfolio value history
                System.out.println("Calculating portfolio value...");
                BigDecimal totalValue = calculatePortfolioValue(portfolio.getId());
                System.out.println("Total portfolio value: " + totalValue);
                
                System.out.println("Creating portfolio value history...");
                PortfolioValueHistory snapshot = new PortfolioValueHistory(portfolio, Instant.now(), totalValue);
                System.out.println("Portfolio value history created: " + snapshot);
                
                System.out.println("Saving portfolio value history...");
                PortfolioValueHistory savedHistory = valueHistoryRepository.save(snapshot);
                System.out.println("Portfolio value history saved with ID: " + savedHistory.getId());
            } catch (Exception e) {
                System.err.println("Error saving portfolio value history: " + e.getMessage());
                e.printStackTrace();
                // Don't fail the entire request if history saving fails
            }
            
            return ResponseEntity.ok(savedTransaction);

        } catch (Exception e) {
            System.err.println("Error placing order: " + e.getMessage());
            e.printStackTrace();
            return ResponseEntity.badRequest().build();
        }
    }

    private BigDecimal calculatePortfolioValue(Long portfolioId) {
        var txns = transactionRepository.findByPortfolioId(portfolioId);
        java.util.Map<String, Integer> holdings = new java.util.HashMap<>();
        
        // Track current holdings for each stock
        for (var txn : txns) {
            String symbol = txn.getStockSymbol();
            int quantity = txn.getQuantity();
            
            if (txn.getType() == TradeType.BUY) {
                // Add to holdings
                holdings.put(symbol, holdings.getOrDefault(symbol, 0) + quantity);
            } else if (txn.getType() == TradeType.SELL) {
                // Subtract from holdings
                holdings.put(symbol, holdings.getOrDefault(symbol, 0) - quantity);
            }
        }
        
        // Calculate current value using real-time prices
        BigDecimal totalValue = BigDecimal.ZERO;
        for (var entry : holdings.entrySet()) {
            String symbol = entry.getKey();
            int quantity = entry.getValue();
            
            if (quantity > 0) {
                // Use current market price (for now, use transaction price as fallback)
                BigDecimal currentPrice = getCurrentStockPrice(symbol);
                BigDecimal positionValue = currentPrice.multiply(BigDecimal.valueOf(quantity));
                totalValue = totalValue.add(positionValue);
                System.out.println("Position: " + symbol + " x" + quantity + " @ $" + currentPrice + " = $" + positionValue);
            }
        }
        
        System.out.println("Total portfolio value: $" + totalValue);
        return totalValue;
    }
    
    private BigDecimal getCurrentStockPrice(String symbol) {
        // For now, use a simple price lookup
        // In a real implementation, this would fetch from market data service
        switch (symbol.toUpperCase()) {
            case "AAPL": return new BigDecimal("213.88");
            case "GOOG": return new BigDecimal("194.08");
            case "MSFT": return new BigDecimal("415.22");
            case "TSLA": return new BigDecimal("248.50");
            case "AMZN": return new BigDecimal("178.12");
            default: return new BigDecimal("100.00");
        }
    }

    public static class OrderRequest {
        private Long portfolioId;
        private String symbol;
        private int quantity;
        private String side;
        private double price;

        // Getters and setters
        public Long getPortfolioId() { return portfolioId; }
        public void setPortfolioId(Long portfolioId) { this.portfolioId = portfolioId; }
        
        public String getSymbol() { return symbol; }
        public void setSymbol(String symbol) { this.symbol = symbol; }
        
        public int getQuantity() { return quantity; }
        public void setQuantity(int quantity) { this.quantity = quantity; }
        
        public String getSide() { return side; }
        public void setSide(String side) { this.side = side; }
        
        public double getPrice() { return price; }
        public void setPrice(double price) { this.price = price; }

        @Override
        public String toString() {
            return "OrderRequest{portfolioId=" + portfolioId + ", symbol='" + symbol + "', quantity=" + quantity + ", side='" + side + "', price=" + price + "}";
        }
    }
} 