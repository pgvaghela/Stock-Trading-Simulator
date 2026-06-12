import React, { useState, useEffect } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';
import { TrendingUp } from 'lucide-react';

const PortfolioChart = ({ portfolioId }) => {
  const [chartData, setChartData] = useState([]);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState('');

  useEffect(() => {
    fetchPortfolioHistory();
  }, [portfolioId]);

  const fetchPortfolioHistory = async () => {
    setIsLoading(true);
    setError('');

    try {
      const response = await fetch(`/api/portfolios/${portfolioId}/value-history`);
      
      if (!response.ok) {
        // No history endpoint or no data
        setChartData([]);
        return;
      }

      const history = await response.json();
      
      if (history.length === 0) {
        // No real history data
        setChartData([]);
        return;
      }

      const formattedData = history.map(item => ({
        time: new Date(item.timestamp).toLocaleTimeString(),
        value: parseFloat(item.value)
      }));

      setChartData(formattedData);

    } catch (err) {
      console.error('Error fetching portfolio history:', err);
      setChartData([]);
    } finally {
      setIsLoading(false);
    }
  };

  const formatCurrency = (value) => {
    return new Intl.NumberFormat('en-US', {
      style: 'currency',
      currency: 'USD',
      minimumFractionDigits: 0,
      maximumFractionDigits: 0
    }).format(value);
  };

  if (isLoading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="text-center">
          <TrendingUp className="w-8 h-8 text-accent-500 animate-pulse mx-auto mb-2" />
          <p className="text-gray-600">Loading chart data...</p>
        </div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="text-center">
          <p className="text-red-600">{error}</p>
        </div>
      </div>
    );
  }

  // Show empty state when no real data
  if (chartData.length === 0) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="text-center">
          <TrendingUp className="w-12 h-12 text-gray-400 mx-auto mb-4" />
          <h3 className="text-lg font-medium text-gray-700 mb-2">No Portfolio History</h3>
          <p className="text-gray-600">Start trading to see your portfolio performance</p>
        </div>
      </div>
    );
  }

  return (
    <div className="h-64">
      <ResponsiveContainer width="100%" height="100%">
        <LineChart data={chartData} margin={{ top: 5, right: 30, left: 20, bottom: 5 }}>
          <CartesianGrid strokeDasharray="3 3" stroke="#f0f0f0" />
          <XAxis 
            dataKey="time" 
            stroke="#6b7280"
            fontSize={12}
            tickLine={false}
            axisLine={false}
          />
          <YAxis 
            stroke="#6b7280"
            fontSize={12}
            tickLine={false}
            axisLine={false}
            tickFormatter={formatCurrency}
          />
          <Tooltip 
            contentStyle={{
              backgroundColor: 'white',
              border: 'none',
              borderRadius: '12px',
              boxShadow: '0 10px 25px -3px rgba(0, 0, 0, 0.1)',
              padding: '12px'
            }}
            formatter={(value) => [formatCurrency(value), 'Portfolio Value']}
            labelStyle={{ color: '#6b7280', fontSize: '12px' }}
          />
          <Line 
            type="monotone" 
            dataKey="value" 
            stroke="#3b82f6" 
            strokeWidth={3}
            dot={false}
            activeDot={{ r: 6, fill: '#3b82f6' }}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
};

export default PortfolioChart; 