import React, { useState, useEffect, useRef } from 'react';
import { TrendingUp, TrendingDown, Minus, Plus, X } from 'lucide-react';

const StockCard = ({ stock, portfolioId, onUpdate }) => {
  const [currentPrice, setCurrentPrice] = useState(stock.price || 0);
  const [priceChange, setPriceChange] = useState(0);
  const [isConnected, setIsConnected] = useState(false);
  const [showBuyModal, setShowBuyModal] = useState(false);
  const [showSellModal, setShowSellModal] = useState(false);
  const [quantity, setQuantity] = useState(1);
  const [isLoading, setIsLoading] = useState(false);
  const wsRef = useRef(null);

  useEffect(() => {
    connectWebSocket();
    return () => {
      if (wsRef.current) {
        wsRef.current.close();
      }
    };
  }, [stock.symbol]);

  const connectWebSocket = () => {
    try {
      // Use native WebSocket instead of STOMP
      const ws = new WebSocket(`ws://${window.location.host}/ws`);
      wsRef.current = ws;
      
      ws.onopen = () => {
        setIsConnected(true);
        console.log('WebSocket connected');
      };
      
      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          // Check if this message is for our stock
          if (data.symbol === stock.symbol) {
            const newPrice = data.price || data.currentPrice || data.lastPrice;
            
            if (newPrice && newPrice !== currentPrice) {
              setPriceChange(newPrice - currentPrice);
              setCurrentPrice(newPrice);
            }
          }
        } catch (error) {
          console.error('Error parsing WebSocket message:', error);
        }
      };
      
      ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        setIsConnected(false);
      };
      
      ws.onclose = () => {
        console.log('WebSocket disconnected');
        setIsConnected(false);
      };
      
    } catch (error) {
      console.error('Error connecting to WebSocket:', error);
      setIsConnected(false);
    }
  };

  const handleBuy = async () => {
    if (quantity <= 0) return;
    
    setIsLoading(true);
    try {
      const response = await fetch('/api/orders', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          portfolioId: portfolioId,
          symbol: stock.symbol,
          quantity: quantity,
          side: 'BUY',
          price: currentPrice
        })
      });

      if (response.ok) {
        setShowBuyModal(false);
        setQuantity(1);
        onUpdate(); // Refresh portfolio data
      } else {
        alert('Failed to buy stock. Please try again.');
      }
    } catch (error) {
      console.error('Error buying stock:', error);
      alert('Error buying stock. Please try again.');
    } finally {
      setIsLoading(false);
    }
  };

  const handleSell = async () => {
    if (quantity <= 0 || quantity > stock.quantity) return;
    
    setIsLoading(true);
    try {
      const response = await fetch('/api/orders', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          portfolioId: portfolioId,
          symbol: stock.symbol,
          quantity: quantity,
          side: 'SELL',
          price: currentPrice
        })
      });

      if (response.ok) {
        setShowSellModal(false);
        setQuantity(1);
        onUpdate(); // Refresh portfolio data
      } else {
        alert('Failed to sell stock. Please try again.');
      }
    } catch (error) {
      console.error('Error selling stock:', error);
      alert('Error selling stock. Please try again.');
    } finally {
      setIsLoading(false);
    }
  };

  const formatCurrency = (amount) => {
    return new Intl.NumberFormat('en-US', {
      style: 'currency',
      currency: 'USD',
      minimumFractionDigits: 2,
      maximumFractionDigits: 2
    }).format(amount);
  };

  const formatNumber = (num) => {
    return new Intl.NumberFormat('en-US').format(num);
  };

  const positionValue = currentPrice * stock.quantity;
  const isPositiveChange = priceChange >= 0;

  return (
    <>
      <div className="luxury-card p-6 hover:shadow-luxury-lg transition-all duration-300">
        {/* Header */}
        <div className="flex items-center justify-between mb-4">
          <div>
            <h3 className="text-xl font-bold text-gray-900">{stock.symbol}</h3>
            <p className="text-sm text-gray-600">{stock.name}</p>
          </div>
          <div className={`flex items-center gap-1 px-2 py-1 rounded-full text-xs font-medium ${
            isPositiveChange ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800'
          }`}>
            {isPositiveChange ? (
              <TrendingUp className="w-3 h-3" />
            ) : (
              <TrendingDown className="w-3 h-3" />
            )}
            {formatCurrency(Math.abs(priceChange))}
          </div>
        </div>

        {/* Price and Quantity */}
        <div className="space-y-3 mb-4">
          <div className="flex justify-between items-center">
            <span className="text-sm text-gray-600">Current Price</span>
            <span className="text-lg font-semibold text-gray-900">
              {formatCurrency(currentPrice)}
            </span>
          </div>
          
          <div className="flex justify-between items-center">
            <span className="text-sm text-gray-600">Quantity</span>
            <span className="text-lg font-semibold text-gray-900">
              {formatNumber(stock.quantity)}
            </span>
          </div>
          
          <div className="flex justify-between items-center">
            <span className="text-sm text-gray-600">Position Value</span>
            <span className="text-lg font-semibold text-gray-900">
              {formatCurrency(positionValue)}
            </span>
          </div>
        </div>

        {/* Connection Status */}
        <div className="flex items-center justify-between text-xs text-gray-500">
          <span>Live Updates</span>
          <div className={`w-2 h-2 rounded-full ${
            isConnected ? 'bg-green-500' : 'bg-red-500'
          }`}></div>
        </div>

        {/* Quick Actions */}
        <div className="flex gap-2 mt-4">
          <button
            onClick={() => setShowBuyModal(true)}
            className="flex-1 bg-green-500 hover:bg-green-600 text-white py-2 px-3 rounded-xl text-sm font-medium transition-colors duration-300 flex items-center justify-center gap-1"
          >
            <Plus className="w-4 h-4" />
            Buy
          </button>
          <button
            onClick={() => setShowSellModal(true)}
            disabled={stock.quantity <= 0}
            className="flex-1 bg-red-500 hover:bg-red-600 disabled:bg-gray-300 disabled:cursor-not-allowed text-white py-2 px-3 rounded-xl text-sm font-medium transition-colors duration-300 flex items-center justify-center gap-1"
          >
            <Minus className="w-4 h-4" />
            Sell
          </button>
        </div>
      </div>

      {/* Buy Modal */}
      {showBuyModal && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
          <div className="luxury-card p-6 max-w-md w-full mx-4">
            <div className="flex justify-between items-center mb-4">
              <h3 className="text-lg font-semibold">Buy {stock.symbol}</h3>
              <button onClick={() => setShowBuyModal(false)} className="text-gray-500 hover:text-gray-700">
                <X className="w-5 h-5" />
              </button>
            </div>
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Quantity</label>
                <input
                  type="number"
                  min="1"
                  value={quantity}
                  onChange={(e) => setQuantity(parseInt(e.target.value) || 1)}
                  className="luxury-input w-full"
                />
              </div>
              <div className="text-sm text-gray-600">
                <p>Price: {formatCurrency(currentPrice)}</p>
                <p>Total: {formatCurrency(currentPrice * quantity)}</p>
              </div>
              <div className="flex gap-2">
                <button
                  onClick={handleBuy}
                  disabled={isLoading}
                  className="flex-1 luxury-button"
                >
                  {isLoading ? 'Buying...' : 'Buy'}
                </button>
                <button
                  onClick={() => setShowBuyModal(false)}
                  className="flex-1 bg-gray-300 hover:bg-gray-400 text-gray-700 py-2 px-4 rounded-2xl transition-colors duration-300"
                >
                  Cancel
                </button>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Sell Modal */}
      {showSellModal && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
          <div className="luxury-card p-6 max-w-md w-full mx-4">
            <div className="flex justify-between items-center mb-4">
              <h3 className="text-lg font-semibold">Sell {stock.symbol}</h3>
              <button onClick={() => setShowSellModal(false)} className="text-gray-500 hover:text-gray-700">
                <X className="w-5 h-5" />
              </button>
            </div>
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Quantity (Max: {stock.quantity})</label>
                <input
                  type="number"
                  min="1"
                  max={stock.quantity}
                  value={quantity}
                  onChange={(e) => setQuantity(Math.min(parseInt(e.target.value) || 1, stock.quantity))}
                  className="luxury-input w-full"
                />
              </div>
              <div className="text-sm text-gray-600">
                <p>Price: {formatCurrency(currentPrice)}</p>
                <p>Total: {formatCurrency(currentPrice * quantity)}</p>
              </div>
              <div className="flex gap-2">
                <button
                  onClick={handleSell}
                  disabled={isLoading || quantity > stock.quantity}
                  className="flex-1 bg-red-500 hover:bg-red-600 disabled:bg-gray-300 text-white py-2 px-4 rounded-2xl transition-colors duration-300"
                >
                  {isLoading ? 'Selling...' : 'Sell'}
                </button>
                <button
                  onClick={() => setShowSellModal(false)}
                  className="flex-1 bg-gray-300 hover:bg-gray-400 text-gray-700 py-2 px-4 rounded-2xl transition-colors duration-300"
                >
                  Cancel
                </button>
              </div>
            </div>
          </div>
        </div>
      )}
    </>
  );
};

export default StockCard; 