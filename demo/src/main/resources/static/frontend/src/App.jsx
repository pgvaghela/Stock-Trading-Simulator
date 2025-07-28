import React, { useState, useEffect } from 'react';
import Dashboard from './components/Dashboard';
import { TrendingUp } from 'lucide-react';

function App() {
  const [portfolioId, setPortfolioId] = useState(null);
  const [isLoading, setIsLoading] = useState(false);

  useEffect(() => {
    // Check if user is already in a portfolio
    const savedPortfolioId = localStorage.getItem('portfolioId');
    
    if (savedPortfolioId) {
      setPortfolioId(savedPortfolioId);
    }
  }, []);

  const startTrading = async () => {
    setIsLoading(true);
    
    try {
      // Generate a unique username
      const uniqueUsername = `trader_${Date.now()}`;
      
      // First create a user
      const userResponse = await fetch('/api/users', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username: uniqueUsername })
      });

      if (!userResponse.ok) {
        const errorText = await userResponse.text();
        throw new Error(`Failed to create user: ${userResponse.status} - ${errorText}`);
      }

      const user = await userResponse.json();

      // Then create a portfolio for that user
      const portfolioResponse = await fetch('/api/portfolios', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ user: { id: user.id } })
      });

      if (!portfolioResponse.ok) {
        const errorText = await portfolioResponse.text();
        throw new Error(`Failed to create portfolio: ${portfolioResponse.status} - ${errorText}`);
      }

      const portfolio = await portfolioResponse.json();
      setPortfolioId(portfolio.id);
      localStorage.setItem('portfolioId', portfolio.id);

    } catch (err) {
      console.error('Error starting trading:', err);
      alert(`Failed to start trading: ${err.message}`);
    } finally {
      setIsLoading(false);
    }
  };

  const handleLogout = () => {
    setPortfolioId(null);
    localStorage.removeItem('portfolioId');
  };

  if (portfolioId) {
    return <Dashboard portfolioId={portfolioId} username="Trader" onLogout={handleLogout} />;
  }

  return (
    <div className="min-h-screen flex items-center justify-center p-4">
      <div className="luxury-card p-8 max-w-md w-full">
        <div className="text-center mb-8">
          <div className="flex justify-center mb-4">
            <div className="bg-gradient-to-r from-accent-500 to-accent-600 p-3 rounded-2xl">
              <TrendingUp className="w-8 h-8 text-white" />
            </div>
          </div>
          <h1 className="text-3xl font-bold text-gray-900 mb-2">Trading Simulator</h1>
          <p className="text-gray-600">Start your trading journey</p>
        </div>
        
        <button
          onClick={startTrading}
          disabled={isLoading}
          className="luxury-button w-full flex items-center justify-center gap-2"
        >
          {isLoading ? (
            <>
              <div className="w-5 h-5 border-2 border-white border-t-transparent rounded-full animate-spin"></div>
              Starting...
            </>
          ) : (
            <>
              <TrendingUp className="w-5 h-5" />
              Start Trading
            </>
          )}
        </button>
      </div>
    </div>
  );
}

export default App; 