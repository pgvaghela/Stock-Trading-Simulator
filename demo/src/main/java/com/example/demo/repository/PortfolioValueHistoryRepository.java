package com.example.demo.repository;

import com.example.demo.entity.PortfolioValueHistory;
import com.example.demo.entity.Portfolio;
import org.springframework.data.jpa.repository.JpaRepository;
import java.time.Instant;
import java.util.List;

public interface PortfolioValueHistoryRepository extends JpaRepository<PortfolioValueHistory, Long> {
    List<PortfolioValueHistory> findByPortfolioOrderByTimestampAsc(Portfolio portfolio);
    List<PortfolioValueHistory> findByPortfolioIdOrderByTimestampAsc(Long portfolioId);
    List<PortfolioValueHistory> findByPortfolioIdAndTimestampAfterOrderByTimestampAsc(Long portfolioId, Instant after);
} 