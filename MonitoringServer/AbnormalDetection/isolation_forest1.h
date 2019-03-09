
#pragma once
#include <stdint.h>
#include <memory>
#include <array>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>
#include <math.h>

namespace iforest
{
	template <typename ValueType>
	class UniformRandomNumber  //产生随机数
	{
	public:
		static ValueType GenerateNext(std::mt19937& rng, ValueType min, ValueType max)
		{
			return ValueType();
		}
	};

	template <>
	class UniformRandomNumber<int32_t>
	{
	public:
		static int32_t GenerateNext(std::mt19937& rng, int32_t min, int32_t max)
		{
			std::uniform_int_distribution<int32_t> value_dis(min, max);
			return value_dis(rng);
		}
	};

	template <>
	class UniformRandomNumber<int64_t>
	{
	public:
		static int64_t GenerateNext(std::mt19937& rng, int64_t min, int64_t max)
		{
			std::uniform_int_distribution<int64_t> value_dis(min, max);
			return value_dis(rng);
		}
	};

	template <>
	class UniformRandomNumber<uint32_t>
	{
	public:
		static uint32_t GenerateNext(std::mt19937& rng, uint32_t min, uint32_t max)
		{
			std::uniform_int_distribution<uint32_t> value_dis(min, max);
			return value_dis(rng);
		}
	};

	template <>
	class UniformRandomNumber<uint64_t>
	{
	public:
		static uint64_t GenerateNext(std::mt19937& rng, uint64_t min, uint64_t max)
		{
			std::uniform_int_distribution<uint64_t> value_dis(min, max);
			return value_dis(rng);
		}
	};

	template <>
	class UniformRandomNumber<float>
	{
	public:
		static float GenerateNext(std::mt19937& rng, float min, float max)
		{
			std::uniform_real_distribution<float> value_dis(min, max);
			return value_dis(rng);
		}
	};

	template <>
	class UniformRandomNumber<double>
	{
	public:
		static double GenerateNext(std::mt19937& rng, double min, double max)
		{
			std::uniform_real_distribution<double> value_dis(min, max);
			return value_dis(rng);
		}
	};

	// 计算H(i)
	static inline double CalculateH(uint32_t i)
	{
		return log(i) + 0.5772156649;
	}

	// 计算c(n)
	static inline double CalculateC(uint32_t n)       
	{
		if (n > 2)
		{
			double h = CalculateH(n - 1);
			return double(2.0 * h) - (double(2.0 * (n - 1)) / double(n));
		}
		else if (n == 2)
		{
			return 1.0;
		}
		else
		{
			return 0.0;
		}
	}

	// 计算异常指数
	static inline double CalculateAnomalyScore(uint32_t sampleCount, double avgPathLen)
	{
		return pow(2, -avgPathLen / CalculateC(sampleCount));
	}


	//孤立树的类设计
	template <typename ValueType, size_t DimensionCount> class IsolationForest;

	template <typename ValueType, size_t DimensionCount>
	class IsolationTree
	{
		static_assert(std::is_default_constructible<ValueType>::value, "ValueType must be default-constructible");
		static_assert(std::is_integral<ValueType>::value || std::is_floating_point<ValueType>::value, "ValueType must be of integral or floating point type");

		static_assert(DimensionCount > 0, "DimensionCount must be greater than zero");

		friend class IsolationForest<ValueType, DimensionCount>;

	public:

		using item_t = std::array<ValueType, DimensionCount>;

	private:

		class Node
		{
			uint32_t m_dimId;
			ValueType m_dimSplitValue;
			uint32_t m_size;
			std::unique_ptr<Node> m_left;
			std::unique_ptr<Node> m_right;

		public:

			Node()
			{
				m_dimId = 0;
				m_dimSplitValue = ValueType();
				m_size = 0;
				m_left = nullptr;
				m_right = nullptr;
			}

			bool Build(std::mt19937& rng, std::vector<item_t*>& data, uint32_t first, uint32_t last, uint32_t depth, uint32_t maxDepth, uint32_t im, std::vector<float>& weight)   //构建孤立树的函数入口
			{
				if (last < first || last >= data.size())
				{
					return false;
				}

				if (last - first < 1 || depth >= maxDepth)
				{
					m_size = (last - first) + 1;
					m_dimId = im;
					return true;
				}

				//std::uniform_int_distribution<uint32_t> dimension_dis(0, DimensionCount - 1);

				//uint32_t dim = m_dimId = dimension_dis(rng);   //这里是随机产生特征值
				

				//随机加权调度算法
				float wAll = 0;
				for(int i = 0;i<6;i++){
					wAll += weight[i];
				}
				std::uniform_real_distribution<float> dimension_dis(0, wAll);  //应该不用-1
				int dim = 6;
				float dis = dimension_dis(rng);
				while(dis<wAll){
					dim--;
					dis += weight[dim];
				}
				m_dimId = dim;
			

				std::sort(data.begin() + first, data.begin() + last + 1, [&dim](const item_t* left, const item_t* right) { return (*left)[dim] < (*right)[dim]; });

				ValueType minVal = (*data[first])[dim];
				ValueType maxVal = (*data[last])[dim];
				uint32_t middle;
				
				//判断是否可以继续划分
				if (minVal == maxVal)
				{
					m_size = (last - first) + 1;
					return true;
				}
				/*{
					m_left = std::unique_ptr<Node>(new Node());
					m_right = std::unique_ptr<Node>(new Node());
					middle = (first+last)/2 + 1;
					if (!m_left->Build(rng, data, first, middle - 1, depth + 1, maxDepth, m_dimId, weight))
					{
						return false;
					}

					if (!m_right->Build(rng, data, middle, last, depth + 1, maxDepth, m_dimId, weight))
					{
						return false;
					}

					return true;
				}*/

				m_dimSplitValue = UniformRandomNumber<ValueType>::GenerateNext(rng, minVal, maxVal);   //产生随机特征值
				middle = first;

				for (middle = first; middle <= last; middle++)
				{
					if ((*data[middle])[dim] >= m_dimSplitValue)
					{
						break;
					}
				}

				if (middle == first)
				{
					m_size = (last - first) + 1;
					return true;
					//middle++;
				}

				//将节点上数据划分到两个子节点上，然后进行递归
				m_left = std::unique_ptr<Node>(new Node());
				//m_left.prem_dimId = m_dimId;
				m_right = std::unique_ptr<Node>(new Node());
				//m_right.prem_dimId = m_dimId;
				if (!m_left->Build(rng, data, first, middle - 1, depth + 1, maxDepth, m_dimId, weight))
				{
					return false;
				}

				if (!m_right->Build(rng, data, middle, last, depth + 1, maxDepth, m_dimId, weight))
				{
					return false;
				}

				return true;
			}

			bool Serialize(std::ostream& os) const
			{
				os.write(reinterpret_cast<const char*>(&m_dimId), sizeof(uint32_t));
				os.write(reinterpret_cast<const char*>(&m_dimSplitValue), sizeof(ValueType));
				os.write(reinterpret_cast<const char*>(&m_size), sizeof(uint32_t));

				if (!os.good())
				{
					return false;
				}

				if (!IsLeaf())
				{
					return m_left->Serialize(os) && m_right->Serialize(os);
				}

				return true;
			}

			bool Deserialize(std::istream& is)
			{				
				is.read(reinterpret_cast<char*>(&m_dimId), sizeof(uint32_t));
				is.read(reinterpret_cast<char*>(&m_dimSplitValue), sizeof(ValueType));
				is.read(reinterpret_cast<char*>(&m_size), sizeof(uint32_t));

				if (!is.good())
				{
					return false;
				}

				if (!m_size)
				{
					m_left = std::unique_ptr<Node>(new Node());
					m_right = std::unique_ptr<Node>(new Node());
					
					return m_left->Deserialize(is) && m_right->Deserialize(is);
				}

				return true;
			}

			bool IsLeaf() const
			{
				return m_left == nullptr || m_right == nullptr;
			}

			double GetPathLen(const item_t& data, uint32_t currentDepth, std::vector<int>& feature) const   //计算路径长度
			{
				//找到孤立节点
				if (IsLeaf())                     
				{
					 //将对应的孤立特征数进行增加
					feature[m_dimId]++; 
					return double(currentDepth) + CalculateC(m_size);   //通过当前节点高度和节点上数据量计算出路径长度
				}

				if (data[m_dimId] < m_dimSplitValue)
				{
					return m_left->GetPathLen(data, currentDepth + 1, feature);
				}
				else
				{
					return m_right->GetPathLen(data, currentDepth + 1, feature);
				}
			}
		};

		uint32_t m_sampleSize;
		std::unique_ptr<Node> m_root;

	public:

		void Clear()
		{
			m_sampleSize = 0;
			m_root.reset();
		}

		bool Build(uint32_t seed, std::vector<item_t>& data, uint32_t sampleSize, std::vector<float>& weight)
		{
			Clear();

			if (!data.size() || !sampleSize || sampleSize > static_cast<uint32_t>(data.size()))
			{
				return false;
			}

			m_sampleSize = sampleSize;

			std::mt19937 gen(seed);

			std::vector<uint32_t> sampleIds;
			sampleIds.reserve(data.size());

			for (uint32_t i = 0; i < static_cast<uint32_t>(data.size()); i++)
			{
				sampleIds.push_back(i);
			}

			std::shuffle(sampleIds.begin(), sampleIds.end(), gen);

			std::vector<item_t*> localData;
			localData.reserve(sampleSize);

			for (uint32_t i = 0; i < sampleSize; i++)    //抽样产生数据集
			{
				//localData.push_back(&data[sampleIds[i]]);    //加入修改，不进行抽样，直接把数据集算进去
				localData.push_back(&data[i]);  
			}

			if (localData.empty())
			{
				return false;
			}

			// The tree height limit maxDepth is automatically set by
			// the subsampling size: maxDepth = ceiling(log2 sampleSize)
			// which is approximately the average tree height
			uint32_t maxDepth = static_cast<uint32_t>(ceil(log2(sampleSize)));

			m_root = std::move(std::unique_ptr<Node>(new Node()));

			return m_root->Build(gen, localData, 0, static_cast<uint32_t>(localData.size()) - 1, 0, maxDepth, 0, weight);
		}

		bool Serialize(std::ostream& os) const
		{
			if (m_root)
			{
				size_t dimCnt = DimensionCount;
				os.write(reinterpret_cast<const char*>(&dimCnt), sizeof(size_t));
				os.write(reinterpret_cast<const char*>(&m_sampleSize), sizeof(uint32_t));

				return m_root->Serialize(os);
			}

			return false;
		}

		bool Deserialize(std::istream& is)
		{
			Clear();

			size_t dimCnt = 0;
			is.read(reinterpret_cast<char*>(&dimCnt), sizeof(size_t));
			is.read(reinterpret_cast<char*>(&m_sampleSize), sizeof(uint32_t));

			if (dimCnt != DimensionCount)
			{
				return false;
			}

			m_root = std::move(std::unique_ptr<Node>(new Node()));						
			return m_root->Deserialize(is);
		}
				
		double GetPathLen(const item_t& data,std::vector<int>& feature) const    //计算路径中第一次调用的
		{
			if (!m_root)
			{
				return 0.0;
			}

			return m_root->GetPathLen(data, 0 ,feature);
		}

		double GetAnomalyScore(const item_t& data) const
		{
			if (!m_root)
			{
				return -1.0;
			}

			return pow(2, -GetPathLen(data) / CalculateC(m_sampleSize));
		}

		bool GetAnomalyScores(const std::vector<item_t>& data, std::vector<double>& scores) const           //这个应该没用到
		{
			scores.clear();

			if (!data.size())
			{
				return false;
			}

			if (!m_root)
			{
				return false;
			}

			scores.resize(data.size());

			for (auto i = 0; i < data.size(); i++)
			{
				scores[i] = pow(2, -GetPathLen(data[i]) / CalculateC(m_sampleSize));
			}

			return true;
		}

		inline size_t GetDimensions() const
		{
			return DimensionCount;
		}
	};

	template <typename ValueType, size_t DimensionCount>
	class IsolationForest
	{
		uint32_t m_sampleSize;
		double m_precalculatedC;

		std::vector<IsolationTree<ValueType, DimensionCount>> m_trees;
				
	public:

		using item_t = std::array<ValueType, DimensionCount>;     //表示一组数据
		//using feature = std::array<uint32_t, DimensionCount>;		//一组数据的孤立特征

		void Clear()
		{
			m_sampleSize = 0;
			m_precalculatedC = 0.0;
			m_trees.clear();
		}

		bool Build(uint32_t treeCount, uint32_t seed, std::vector<item_t>& data, uint32_t sampleSize, std::vector<float>& weight)  //构造孤立森林的起始函数
		{
			Clear();

			if (!data.size() || !sampleSize || sampleSize > static_cast<uint32_t>(data.size()))
			{
				return false;
			}

			m_sampleSize = sampleSize;
			m_precalculatedC = CalculateC(m_sampleSize);
			
			std::mt19937 gen(seed);
			std::uniform_int_distribution<uint32_t> uniform_dis(0, std::numeric_limits<uint32_t>::max());

			m_trees.resize(treeCount);   //初始化孤立树，一个数组

			for (uint32_t i = 0; i < treeCount; i++)   //循环构造孤立树
			{
				if (!m_trees[i].Build(uniform_dis(gen), data, sampleSize, weight))  //这里调用构造孤立树的私有方法
				{
					return false;
				}
			}

			return true;
		}

		double GetAnomalyScore(const item_t& data) const  //获取异常指数
		{
			double totalPathLen = 0;

			if (m_trees.empty())
			{
				return -1.0;
			}

			for (const auto& t : m_trees)  //累加计算总长度
			{
				totalPathLen += t.GetPathLen(data);  //私有的计算路径长度
			}

			double avgPathLen = totalPathLen / double(m_trees.size()); //

			return pow(2, -avgPathLen / m_precalculatedC);
		}

		bool GetAnomalyScores(const std::vector<item_t>& data, std::vector<double>& scores,std::vector<std::vector<int>>& features) const   //检查是否能计算出异常指数,实际调用的就是这个
		{
			scores.clear();
			for (int i = 0; i < data.size(); i++){  //将孤立特征组置0
				for(int j = 0;j < 6;j++){
					features[i][j] = 0;
				}
			}
			if (!data.size())
			{
				return false;
			}

			if (m_trees.empty())
			{
				return false;
			}

			scores.resize(data.size());

			for (auto i = 0; i < data.size(); i++)   //遍历所有节点
			{
				double totalPathLen = 0;

				for (const auto& t : m_trees)
				{
					totalPathLen += t.GetPathLen(data[i], features[i]);   //遍历孤立树
				}

				double avgPathLen = totalPathLen / double(m_trees.size());

				scores[i] = pow(2, -avgPathLen / m_precalculatedC);           //计算异常指数
			}

			return true;
		}

		bool Serialize(std::ostream& os) const
		{
			if (!m_sampleSize || !m_trees.size())
			{
				return false;
			}

			uint32_t treeCount = static_cast<uint32_t>(m_trees.size());
			
			size_t dimCnt = DimensionCount;
			os.write(reinterpret_cast<const char*>(&dimCnt), sizeof(size_t));
			os.write(reinterpret_cast<const char*>(&treeCount), sizeof(uint32_t));
			os.write(reinterpret_cast<const char*>(&m_sampleSize), sizeof(uint32_t));
			
			if (!os.good())
			{
				return false;
			}

			for (uint32_t i = 0; i < treeCount; i++)
			{
				if (!m_trees[i].Serialize(os))
				{
					return false;
				}
			}

			return true;
		}

		bool Deserialize(std::istream& is)
		{
			Clear();

			uint32_t treeCount = 0;
			size_t dimCnt = 0;
			
			is.read(reinterpret_cast<char*>(&dimCnt), sizeof(size_t));
			is.read(reinterpret_cast<char*>(&treeCount), sizeof(uint32_t));
			is.read(reinterpret_cast<char*>(&m_sampleSize), sizeof(uint32_t));

			if (dimCnt != DimensionCount)
			{
				return false;
			}

			m_precalculatedC = CalculateC(m_sampleSize);

			if (!is.good())
			{
				return false;
			}

			m_trees.resize(treeCount);

			for (uint32_t i = 0; i < treeCount; i++)
			{
				if (!m_trees[i].Deserialize(is))
				{
					return false;
				}
			}

			return true;
		}

		size_t GetDimensions() const
		{
			return DimensionCount;
		}
	};
};
