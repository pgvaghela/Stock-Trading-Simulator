import React, { useState, useEffect } from 'react';
import StockCard from './StockCard';
import PortfolioChart from './PortfolioChart';
import { LogOut, TrendingUp, DollarSign, RefreshCw } from 'lucide-react';

const Dashboard = ({ portfolioId, username, onLogout }) => {
  const [stocks, setStocks] = useState([]);
  const [allStocks, setAllStocks] = useState([]);
  const [transactions, setTransactions] = useState([]);
  const [totalValue, setTotalValue] = useState(0);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState('');

  useEffect(() => {
    fetchPortfolioData();
    fetchAllStocks();
  }, [portfolioId]);

  const fetchAllStocks = async () => {
    try {
      const response = await fetch('/api/stocks');
      if (response.ok) {
        const stocksData = await response.json();
        setAllStocks(stocksData);
      }
    } catch (err) {
      console.error('Error fetching all stocks:', err);
    }
  };

  const fetchPortfolioData = async () => {
    setIsLoading(true);
    setError('');

    try {
      // Fetch transactions
      const transactionsResponse = await fetch(`/api/portfolios/${portfolioId}/transactions`);
      if (!transactionsResponse.ok) {
        throw new Error('Failed to fetch transactions');
      }
      const transactionsData = await transactionsResponse.json();
      setTransactions(transactionsData);

      // Calculate holdings and fetch stock data
      const holdings = calculateHoldings(transactionsData);
      const stockSymbols = Object.keys(holdings);
      
      // Fetch stock data for all holdings
      const stockPromises = stockSymbols.map(async (symbol) => {
        const response = await fetch(`/api/stocks/${symbol}`);
        if (response.ok) {
          const stock = await response.json();
          return {
            ...stock,
            quantity: holdings[symbol],
            value: (stock.price || 0) * holdings[symbol]
          };
        }
        return null;
      });

      const stockData = await Promise.all(stockPromises);
      const validStocks = stockData.filter(stock => stock !== null);
      setStocks(validStocks);

      // Calculate total value
      const total = validStocks.reduce((sum, stock) => sum + stock.value, 0);
      setTotalValue(total);

    } catch (err) {
      setError(err.message);
    } finally {
      setIsLoading(false);
    }
  };

  const calculateHoldings = (transactions) => {
    const holdings = {};
    
    transactions.forEach(txn => {
      const symbol = txn.stockSymbol; // Use the correct field name
      const quantity = txn.type === 'BUY' ? txn.quantity : -txn.quantity;
      
      if (symbol) {
        holdings[symbol] = (holdings[symbol] || 0) + quantity;
      }
    });

    // Remove stocks with 0 or negative holdings
    return Object.fromEntries(
      Object.entries(holdings).filter(([_, qty]) => qty > 0)
    );
  };

  const formatCurrency = (amount) => {
    return new Intl.NumberFormat('en-US', {
      style: 'currency',
      currency: 'USD',
      minimumFractionDigits: 2,
      maximumFractionDigits: 2
    }).format(amount);
  };

  if (isLoading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="luxury-card p-8 text-center">
          <RefreshCw className="w-8 h-8 text-accent-500 animate-spin mx-auto mb-4" />
          <p className="text-gray-600">Loading your portfolio...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen p-4">
      {/* Header */}
      <div className="luxury-card p-6 mb-6">
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-2xl font-bold text-gray-900">Welcome back, {username}</h1>
            <p className="text-gray-600">Portfolio ID: {portfolioId}</p>
          </div>
          <button
            onClick={onLogout}
            className="bg-red-500 hover:bg-red-600 text-white px-4 py-2 rounded-2xl transition-colors duration-300 flex items-center gap-2"
          >
            <LogOut className="w-4 h-4" />
            Logout
          </button>
        </div>
      </div>

      {/* Total Value Card */}
      <div className="luxury-card p-6 mb-6">
        <div className="flex items-center justify-between">
          <div>
            <h2 className="text-lg font-medium text-gray-700 mb-1">Total Portfolio Value</h2>
            <p className="text-3xl font-bold text-gray-900">{formatCurrency(totalValue)}</p>
          </div>
          <div className="bg-gradient-to-r from-green-500 to-green-600 p-3 rounded-2xl">
            <DollarSign className="w-8 h-8 text-white" />
          </div>
        </div>
      </div>

      {/* Portfolio Chart */}
      <div className="luxury-card p-6 mb-6">
        <h3 className="text-lg font-medium text-gray-700 mb-4">Portfolio Performance</h3>
        <PortfolioChart portfolioId={portfolioId} />
      </div>

      {/* Error Display */}
      {error && (
        <div className="luxury-card p-4 mb-6 bg-red-50 border border-red-200">
          <p className="text-red-600">{error}</p>
        </div>
      )}

      {/* Your Holdings */}
      {stocks.length > 0 && (
        <div className="mb-8">
          <h3 className="text-xl font-semibold text-gray-900 mb-4">Your Holdings</h3>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
            {stocks.map((stock) => (
              <StockCard
                key={stock.symbol}
                stock={stock}
                portfolioId={portfolioId}
                onUpdate={fetchPortfolioData}
              />
            ))}
          </div>
        </div>
      )}

      {/* Available Stocks for Trading */}
      <div className="mb-8">
        <h3 className="text-xl font-semibold text-gray-900 mb-4">Available Stocks</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          {allStocks.map((stock) => {
            // Find if user has holdings in this stock
            const userStock = stocks.find(s => s.symbol === stock.symbol);
            const stockWithQuantity = userStock || { ...stock, quantity: 0, value: 0 };
            
            return (
              <StockCard
                key={stock.symbol}
                stock={stockWithQuantity}
                portfolioId={portfolioId}
                onUpdate={fetchPortfolioData}
              />
            );
          })}
        </div>
      </div>

      {allStocks.length === 0 && !isLoading && (
        <div className="luxury-card p-8 text-center">
          <TrendingUp className="w-12 h-12 text-gray-400 mx-auto mb-4" />
          <h3 className="text-lg font-medium text-gray-700 mb-2">No stocks available</h3>
          <p className="text-gray-600">Check back later for available stocks</p>
        </div>
      )}
    </div>
  );
};

export default Dashboard; 