import React, { useState, useEffect } from "react";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
} from "recharts";
import axios from "axios";

interface DataPoint {
  timestamp: string;
  waterLevel: number;
}

const App: React.FC = () => {

    const [data, setData] = useState<DataPoint[]>([]);
    const [threshold, setThreshold] = useState<number>(50);

    const isAboveThreshold = data[data.length - 1]?.waterLevel > threshold;

    useEffect(() => {
        const fetchData = async () => {
            try {
              // Fetch data from the backend
              const response = await axios.get("http://127.0.0.1:3000/api/water-level");

              // If the API returns a single object, add it to the state
              if (Array.isArray(response.data)) {
                setData(response.data); // Replace all data
                console.log(data)
              } else {
                const newPoint: DataPoint = response.data;
                setData((prevData) => [...prevData, newPoint]); // Append new point
                console.log(data)
              }
            } catch (error) {
              console.error("Error fetching water level data:", error);
            }
          };


      // Fetch data every 2 seconds
      const intervalId = setInterval(fetchData, 1000);

      return () => clearInterval(intervalId); // Cleanup interval on component unmount
    }, []);

//   const [data, setData] = useState<DataPoint[]>([]);
//   const [threshold, setThreshold] = useState<number>(50);

//   const isAboveThreshold = data[data.length - 1]?.waterLevel > threshold;

//   useEffect(() => {
//     // Static data for testing
//     setData([
//       { timestamp: "12:00 PM", waterLevel: 10 },
//       { timestamp: "12:01 PM", waterLevel: 20 },
//       { timestamp: "12:02 PM", waterLevel: 30 },
//       { timestamp: "12:03 PM", waterLevel: 25 },
//       { timestamp: "12:04 PM", waterLevel: 15 },
//     ]);
//   }, []);

  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-800 via-gray-900 to-black text-white flex flex-col items-center p-8">
      {/* Header */}
      <h1 className="text-4xl font-extrabold mb-8 text-center bg-clip-text text-transparent bg-gradient-to-r from-highlight-blue via-highlight-green to-highlight-red">
        Water Level Monitor
      </h1>

      {/* Card Section */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-8 w-full max-w-6xl">
        {/* Water Level Card */}
        <div
          className={`p-6 rounded-xl shadow-lg border ${
            isAboveThreshold
              ? "bg-gradient-to-br from-red-800 via-red-700 to-red-500 border-red-500"
              : "bg-gradient-to-br from-card-bg via-gray-800 to-gray-700 border-card-border"
          }`}
        >
          <h2 className="text-2xl font-bold mb-4">Current Water Level</h2>
          <p className="text-5xl font-extrabold">
            {data[data.length - 1]?.waterLevel || 0}
          </p>
          {isAboveThreshold ? (
            <p className="text-red-300 font-medium mt-4">
              Warning: Water level exceeds the threshold!
            </p>
          ) : (
            <p className="text-highlight-green font-medium mt-4">
              Water level is within safe limits.
            </p>
          )}
        </div>

        {/* Graph Card */}
        <div className="p-6 bg-gradient-to-br from-card-bg via-gray-800 to-gray-700 rounded-xl shadow-lg border border-card-border">
          <h2 className="text-2xl font-bold mb-4">Live Water Level Graph</h2>
          <LineChart
            width={500}
            height={300}
            data={data}
            className="bg-gray-800 rounded-lg p-4"
          >
            <CartesianGrid strokeDasharray="3 3" stroke="#374151" />
            <XAxis dataKey="timestamp" stroke="#CBD5E1" />
            <YAxis stroke="#CBD5E1" />
            <Tooltip />
            <Line
              type="monotone"
              dataKey="waterLevel"
              stroke="#3B82F6"
              strokeWidth={3}
              activeDot={{ r: 8 }}
            />
          </LineChart>
        </div>
      </div>
    </div>
  );
};

export default App;
