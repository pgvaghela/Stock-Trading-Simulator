package com.example.demo.controller;

import java.util.List;

import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.example.demo.entity.Portfolio;
import com.example.demo.entity.Transaction;
import com.example.demo.repository.PortfolioRepository;
import com.example.demo.repository.TransactionRepository;

@RestController
@RequestMapping("/api/portfolios")
@Component
public class PortfolioController {
    private final PortfolioRepository portfolioRepo;
    private final TransactionRepository txnRepo;

    public PortfolioController(PortfolioRepository portfolioRepo, TransactionRepository txnRepo) {
        this.portfolioRepo = portfolioRepo;
        this.txnRepo = txnRepo;
    }

    @PostMapping
    public ResponseEntity<Portfolio> createOrFetch(@RequestBody Portfolio req) {
        Long userId = req.getUser() != null ? req.getUser().getId() : null;
        if (userId == null) {
            return ResponseEntity.badRequest().build();
        }
        List<Portfolio> existing = portfolioRepo.findByUserId(userId);
        if (!existing.isEmpty()) return ResponseEntity.ok(existing.get(0));
        return ResponseEntity.ok(portfolioRepo.save(req));
    }

    @GetMapping("/{id}")
    public ResponseEntity<Portfolio> getPortfolio(@PathVariable Long id) {
        return portfolioRepo.findById(id)
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.notFound().build());
    }

    @GetMapping("/{id}/transactions")
    public ResponseEntity<List<Transaction>> getTransactions(@PathVariable Long id) {
        try {
            System.out.println("Fetching transactions for portfolio ID: " + id);
            
            // Check if portfolio exists
            if (!portfolioRepo.existsById(id)) {
                System.out.println("Portfolio not found: " + id);
                return ResponseEntity.notFound().build();
            }
            
            List<Transaction> transactions = txnRepo.findByPortfolioId(id);
            System.out.println("Found " + transactions.size() + " transactions");
            
            return ResponseEntity.ok(transactions);
        } catch (Exception e) {
            System.err.println("Error fetching transactions: " + e.getMessage());
            e.printStackTrace();
            return ResponseEntity.internalServerError().build();
        }
    }
}
