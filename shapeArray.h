#include<iostream>
#include<vector>
#include<math.h>

template <class T> class ShapeArray
{
private:
	std::vector<int> mshape;
	T* mdata;
	long msize;
public:
	ShapeArray() {
		mdata = NULL;
		msize = 0;
		mshape.push_back(0);
	}

	ShapeArray(const T* data, long size) {
		mdata = new T[size];
		memcpy(mdata, data, size * sizeof(T));
		msize = size;
		mshape.push_back(0);
	}

	~ShapeArray() {
		if (mdata != NULL) {
			delete[] mdata;
		}
		mdata = NULL;
		msize = 0;
		mshape.clear();
	}

	bool reshape(const std::vector<int>& shape) {
		int index = -1;
		const int flag = -1;
		for (int i = 0; i < shape.size(); ++i) {
			if (shape[i] == flag) {
				index = i;
				break;
			}
		}
		std::vector<int> newShape;
		newShape.assign(shape.begin(), shape.end());
		if (index != -1) {
			int tmp = msize;
			for (int i = 0; i < shape.size(); ++i) {
				if (i != index) {
					tmp = tmp / shape[i];
				}
			}
			newShape[index] = tmp;
		}

		long rsize = 1;
		for (int i = 0; i < newShape.size(); ++i) {
			rsize = rsize * newShape[i];
		}
		if (rsize != msize) {
			return false;
		}
		mshape.assign(newShape.begin(), newShape.end());
		return true;
	}
	void setData(const T* data, long size) {
		if (mdata != NULL) {
			delete[] mdata;
		}
		mdata = new T[size];
		memcpy(mdata, data, size * sizeof(T));
		msize = size;
                mshape.clear();
                mshape.push_back(msize);
	}

	void print() {
		printArray(mdata, msize, 0);

		std::cout << "size=" << msize << " shape is (";
		for (int i = 0; i < mshape.size(); ++i) {
			std::cout << mshape[i] << " ";
		}
		std::cout << ")\n" << std::endl;
	}

	void clear() {
		if (mdata != NULL) {
			delete[] mdata;
		}
		mdata = NULL;
		msize = 0;
		mshape.clear();
		mshape.push_back(1);
	}

	void arange(T num) {
		return arange(0, num, 1);
	}

	void arange(T start, T stop) {
		return arange(start, stop, 1);
	}

	void arange(T start, T stop, T step) {
		clear();
		if (step <= 0) {
			return;
		}
		if (stop <= start) {
			return;
		}
		if ((stop - start) < step) {
			return;
		}
		msize = (stop - start) / step;
		mdata = new T[msize];
		for (int i = 0; i < msize; ++i) {
			mdata[i] = start + i * step;
		}
		mshape[0]=msize;
	}

	void repeat(int repeats) {
		long size = repeats * msize;
		T* data = new T[size];
		for (int i = 0; i < msize; ++i) {
			for (int j = 0; j < repeats; ++j) {
				data[i*repeats + j] = mdata[i];
			}
		}
		clear();
		msize = size;
		mdata = data;
	}

	void repeat(const std::vector<int>& repeatArray, int axis) {
		if (repeatArray.empty()) {
			return;
		}
		if ((axis >= 0) && (axis >= mshape.size())) {
			return;
		}
		if ((axis < 0) && ((axis + mshape.size()) < 0)) {
			return;
		}
		int index = (mshape.size() + axis) % mshape.size();
		std::vector<int> shape;
		shape.assign(mshape.begin(), mshape.end());
		if ((repeatArray.size() != 1) && (repeatArray.size() != mshape[index])) {
			return;
		}
		if (repeatArray.size() == 1) {
			shape[index] = shape[index] * repeatArray[0];
		}
		else {
			shape[index] = 0;
			for (int i = 0; i < repeatArray.size(); ++i) {
				shape[index] += repeatArray[i];
			}
		}

		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		T* data = new T[size];

		int step = msize;
		int arrayNum = msize;

		for (int i = 0; i < index; i++) {
			arrayNum = arrayNum / mshape[i];
		}

		for (int i = 0; i <= index; i++) {
			step = step / mshape[i];
		}

		int times = msize / arrayNum;
		int subtimes = arrayNum / step;
		std::vector<int> tmpRepeat;
		if (repeatArray.size() == 1) {
			for (int i = 0; i < mshape[index]; ++i) {
				tmpRepeat.push_back(repeatArray[0]);
			}
		}
		else {
			tmpRepeat.assign(repeatArray.begin(), repeatArray.end());
		}
		long location = 0;
		for (int i = 0; i < times; ++i) {
			for (int j = 0; j < subtimes; ++j) {
				for (int k = 0; k < tmpRepeat[j]; ++k) {
					memcpy((void*)(data + location), (void*)(mdata + i * arrayNum + j * step), step * sizeof(T));
					location += step;
				}
			}
		}
		clear();
		mdata = data;
		msize = size;
		mshape.assign(shape.begin(), shape.end());
	}
	void broadcast(const std::vector<int>& shape) {
		if (shape.size() < mshape.size()) {
			return;
		}
		else if (shape.size() == mshape.size()) {
			bool same = true;
			for (int i = 0; i < shape.size(); ++i) {
				if (shape[i] != mshape[i]) {
					same = false;
					break;
				}
			}
			if (same) {
				return;
			}
		}
		int boradnum = shape.size() - mshape.size();
		std::vector<int> broadShape;
		for (int i = 0; i < boradnum; ++i) {
			broadShape.push_back(shape[i]);
		}
		for (int i = 0; i < mshape.size(); ++i) {
			if (mshape[i] == shape[i + boradnum]) {
				broadShape.push_back(1);
			}
			else if (mshape[i] == 1) {
				broadShape.push_back(shape[i + boradnum]);
			}
			else {
				return;
			}
		}
		tile(broadShape);
	}

	void tile(const std::vector<int>& tileArray) {
		if (tileArray.empty()) {
			return;
		}
		if (tileArray.size() > mshape.size()) {
			int insert = tileArray.size() - mshape.size();
			for (int i = 0; i < insert; ++i) {
				mshape.insert(mshape.begin(), 1);
			}
		}
		std::vector<int>tmpTileArray;
		if (tileArray.size() == mshape.size()) {
			tmpTileArray.assign(tileArray.begin(), tileArray.end());
		}
		else {
			for (int i = 0; i < (mshape.size() - tileArray.size()); ++i) {
				tmpTileArray.push_back(1);
			}
			tmpTileArray.insert(tmpTileArray.end(), tileArray.begin(), tileArray.end());
		}
		std::vector<int> shape;
		shape.assign(mshape.begin(), mshape.end());
		for (int i = 0; i < tmpTileArray.size(); ++i) {
			shape[i] = shape[i] * tmpTileArray[i];
		}
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		T* data = new T[size];
		int last = tmpTileArray[tmpTileArray.size() - 1];
		int arrayNum = msize;

		for (int i = 0; i < mshape.size() - 1; i++) {
			arrayNum = arrayNum / mshape[i];
		}
		int times = msize / arrayNum;
		long location = 0;
		for (int i = 0; i < times; ++i) {
			for (int j = 0; j < last; ++j) {
				memcpy((void*)(data + location), (void*)(mdata + i * arrayNum), arrayNum * sizeof(T));
				location += arrayNum;
			}
		}
		int cols = msize * last;
		times = size / cols;
		for (int i = 0; i < times-1; ++i) {
			memcpy((void*)(data + location), (void*)data, cols * sizeof(T));
			location += cols;
		}

		clear();
		mdata = data;
		msize = size;
		mshape.assign(shape.begin(), shape.end());
	}
	bool isSameDim(const std::vector<int>& shape) {
		if (mshape.size() != shape.size()) {
			return false;
		}
		for (int i = 0; i < mshape.size(); ++i) {
			if (mshape[i] != shape[i]) {
				return false;
			}
		}
		return true;
	}

	const std::vector<int>& getShape() {
		return mshape;
	}

	long getSize() {
		return msize;
	}

	const T* getData() {
		return mdata;
	}
	void printLayer(int layer) {
		return printLayer(mdata, mshape, layer);
	}
	
	
	void concatenate(ShapeArray<T>& concatArray, int axis = 0) {
		if ((axis >= 0) && (axis >= mshape.size())) {
			return;
		}
		if ((axis < 0) && ((axis + mshape.size()) < 0)) {
			return;
		}
		if (mshape.size() != concatArray.getShape().size()) {
			return;
		}
		int index = (mshape.size() + axis) % mshape.size();
		for (int i = 0; i < mshape.size(); ++i) {
			if ((i != index) && (mshape[i] != concatArray.getShape()[i])) {
				return;
			}
		}
		std::vector<int> shape;
		shape.assign(mshape.begin(), mshape.end());
		shape[index] += concatArray.getShape()[index];
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		T* data = new T[size];

		int times = 1;
		for (int i = 0; i < index; ++i) {
			times *= mshape[i];
		}
		int col1 = msize / times;
		int col2 = concatArray.getSize() / times;
		int location = 0;

		for (int i = 0; i < times; ++i) {
			memcpy((void*)(data + location), (void*)(mdata + i * col1), col1 * sizeof(T));
			location += col1;
			memcpy((void*)(data + location), (void*)(concatArray.getData() + i * col2), col2 * sizeof(T));
			location += col2;
		}
		clear();
		mdata = data;
		msize = size;
		mshape.assign(shape.begin(), shape.end());
	}

	void argmax(int axis, ShapeArray<T>& maxArray) {
		if ((axis >= 0) && (axis >= mshape.size())) {
			return;
		}
		if ((axis < 0) && ((axis + mshape.size()) < 0)) {
			return;
		}
		int layer = (mshape.size() + axis) % mshape.size();
		std::vector<int> maxVec;
		argmax(mdata, mshape, layer, maxVec);
		std::vector<int> shape;
		for (int i = 0; i < mshape.size(); ++i) {
			if (i != layer) {
				shape.push_back(mshape[i]);
			}
		}
 		if(shape.size() == 0) {
			shape.push_back(1);		
		}
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		if (size != maxVec.size()) {
			return;
		}
		T *data = new T[size];
		for (int i = 0; i < maxVec.size(); ++i) {
			data[i] = maxVec[i];
		}
		maxArray.clear();
		maxArray.setData(data, size);
		maxArray.reshape(shape);
		delete[] data;
	}

	void multiply_reduce(int axis, ShapeArray<T>& rltArray) {
		if (mshape.size() == 1) {
			return;
		}
		if ((axis >= 0) && (axis >= mshape.size())) {
			return;
		}
		if ((axis < 0) && ((axis + mshape.size()) < 0)) {
			return;
		}
		int layer = (mshape.size() + axis) % mshape.size();
		std::vector<T> outVec;
		multiply_reduce(mdata, mshape, layer, outVec);
		std::vector<int> shape;
		for (int i = 0; i < mshape.size(); ++i) {
			if (i != layer) {
				shape.push_back(mshape[i]);
			}
		}
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		if (size != outVec.size()) {
			return;
		}
		T *data = new T[size];
		for (int i = 0; i < outVec.size(); ++i) {
			data[i] = outVec[i];
		}
		rltArray.clear();
		rltArray.setData(data, size);
		rltArray.reshape(shape);
		delete[] data;
	}

	void minimum(ShapeArray<T>& cmpArray, ShapeArray<T>& rltArray) {
		if (!cmpArray.isSameDim(mshape)) {
			std::cout << "do you want broadcast this array?" << std::endl;
			return;
		}
		rltArray.clear();
		T* data = new T[msize];
		const T* cmpdata = cmpArray.getData();
		for (int i = 0; i < msize; ++i) {
			data[i] = (mdata[i] < cmpdata[i] ? mdata[i]: cmpdata[i]);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

	void maximum(ShapeArray<T>& cmpArray, ShapeArray<T>& rltArray) {
		if (!cmpArray.isSameDim(mshape)) {
			std::cout << "do you want broadcast this array?" << std::endl;
			return;
		}
		rltArray.clear();
		T* data = new T[msize];
		const T* cmpdata = cmpArray.getData();
		for (int i = 0; i < msize; ++i) {
			data[i] = (mdata[i] > cmpdata[i] ? mdata[i] : cmpdata[i]);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

        void maximum(T num, ShapeArray<T>& rltArray) {		
		rltArray.clear();
		T* data = new T[msize];
		for (int i = 0; i < msize; ++i) {
			data[i] = (mdata[i] > num ? mdata[i] : num);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}
	
	void logical_and(ShapeArray<T>& cmpArray, ShapeArray<T>& rltArray) {
		if (!cmpArray.isSameDim(mshape)) {
			return;
		}
		rltArray.clear();
		T* data = new T[msize];
		const T* cmpdata = cmpArray.getData();
		for (int i = 0; i < msize; ++i) {
			data[i] = ((mdata[i] && cmpdata[i]) ? 1:0);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

        void greater(ShapeArray<T>& cmpArray, ShapeArray<T>& rltArray) {
		if (!cmpArray.isSameDim(mshape)) {
			return;
		}
		rltArray.clear();
		T* data = new T[msize];
		const T* cmpdata = cmpArray.getData();
		for (int i = 0; i < msize; ++i) {
			data[i] = ((mdata[i] > cmpdata[i]) ? 1:0);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

        void greater(T num, ShapeArray<T>& rltArray) {
		rltArray.clear();
		T* data = new T[msize];
		for (int i = 0; i < msize; ++i) {
			data[i] = ((mdata[i] > num) ? 1:0);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

       void less(ShapeArray<T>& cmpArray, ShapeArray<T>& rltArray) {
		if (!cmpArray.isSameDim(mshape)) {
			return;
		}
		rltArray.clear();
		T* data = new T[msize];
		const T* cmpdata = cmpArray.getData();
		for (int i = 0; i < msize; ++i) {
			data[i] = ((mdata[i] < cmpdata[i]) ? 1:0);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

        void less(T num, ShapeArray<T>& rltArray) {
		rltArray.clear();
		T* data = new T[msize];
		for (int i = 0; i < msize; ++i) {
			data[i] = ((mdata[i] < num) ? 1:0);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

        void equal(T num, ShapeArray<T>& rltArray) {
		rltArray.clear();
		T* data = new T[msize];
		for (int i = 0; i < msize; ++i) {
			data[i] = ((mdata[i] == num) ? 1:0);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}



	void logical_or(ShapeArray<T>& cmpArray, ShapeArray<T>& rltArray) {
		if (!cmpArray.isSameDim(mshape)) {
			return;
		}
		rltArray.clear();
		T* data = new T[msize];
		const T* cmpdata = cmpArray.getData();
		for (int i = 0; i < msize; ++i) {
			data[i] = ((mdata[i] || cmpdata[i]) ? 1:0);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

	void sqrt(ShapeArray<float>& rltArray) {
		rltArray.clear();
		float *data = new float[msize];
		for (int i = 0; i < msize; ++i) {
			data[i] = std::sqrt(mdata[i]);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

	void exp(ShapeArray<float>& rltArray) {
		rltArray.clear();
		float *data = new float[msize];
		for (int i = 0; i < msize; ++i) {
			data[i] = std::exp(mdata[i]);
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

	void sigmoid(ShapeArray<float>& rltArray) {
		rltArray.clear();
		float *data = new float[msize];
		for (int i = 0; i < msize; ++i) {
			data[i] = 1.0 / (1.0 + std::exp(-mdata[i]));
		}
		rltArray.setData(data, msize);
		rltArray.reshape(mshape);
		delete[] data;
	}

	void add(ShapeArray<T>& anArray) {
		int dims = (mshape.size() < anArray.getShape().size() ? mshape.size() : anArray.getShape().size());
		for (int i = 0; i < dims; ++i) {
			if ((mshape[mshape.size() - 1 - i] != 1)
				&& (anArray.getShape()[anArray.getShape().size() - 1 - i] != 1)
				&& (mshape[mshape.size() - 1 - i] != anArray.getShape()[anArray.getShape().size() - 1 - i])) {
				return;
			}
		}
		std::vector<int> lastShape;
		int index0, index1;
		int maxsize = (mshape.size() > anArray.getShape().size() ? mshape.size() : anArray.getShape().size());
		for(int i = 0; i < maxsize; ++i){
			index1 = anArray.getShape().size() - 1 - i;
			index0 = mshape.size() - 1 - i;
			if (index0 >= 0) {
				if (index1 >= 0) {
					if (mshape[index0] != 1) {
						lastShape.insert(lastShape.begin(), mshape[index0]);
					}
					else {
						lastShape.insert(lastShape.begin(), anArray.getShape()[index1]);
					}
				}
				else {
					lastShape.insert(lastShape.begin(), mshape[index0]);
				}
			}
			else {
				lastShape.insert(lastShape.begin(), anArray.getShape()[index1]);
			}
		}
		bool needBroad = false;
		for (int i = 0; i < lastShape.size(); i++) {
			if (mshape[i] != lastShape[i]) {
				needBroad = true;
				break;
			}
		}
		if (needBroad) {
			broadcast(lastShape);
			needBroad = false;
		}

		for (int i = 0; i < lastShape.size(); i++) {
			if (anArray.getShape()[i] != lastShape[i]) {
				needBroad = true;
				break;
			}
		}

		if (needBroad) {
			ShapeArray<T> cpArray(anArray.getData(), anArray.getSize());
			cpArray.reshape(anArray.getShape());
			cpArray.broadcast(lastShape);
			needBroad = false;
			for (int i = 0; i < msize; ++i) {
				mdata[i] += cpArray.getData()[i];
			}
		}
		else {
			for (int i = 0; i < msize; ++i) {
				mdata[i] += anArray.getData()[i];
			}
		}		
	}

        void subtract(ShapeArray<T>& anArray) {
		int dims = (mshape.size() < anArray.getShape().size() ? mshape.size() : anArray.getShape().size());
		for (int i = 0; i < dims; ++i) {
			if ((mshape[mshape.size() - 1 - i] != 1)
				&& (anArray.getShape()[anArray.getShape().size() - 1 - i] != 1)
				&& (mshape[mshape.size() - 1 - i] != anArray.getShape()[anArray.getShape().size() - 1 - i])) {
				return;
			}
		}
		std::vector<int> lastShape;
		int index0, index1;
		int maxsize = (mshape.size() > anArray.getShape().size() ? mshape.size() : anArray.getShape().size());
		for(int i = 0; i < maxsize; ++i){
			index1 = anArray.getShape().size() - 1 - i;
			index0 = mshape.size() - 1 - i;
			if (index0 >= 0) {
				if (index1 >= 0) {
					if (mshape[index0] != 1) {
						lastShape.insert(lastShape.begin(), mshape[index0]);
					}
					else {
						lastShape.insert(lastShape.begin(), anArray.getShape()[index1]);
					}
				}
				else {
					lastShape.insert(lastShape.begin(), mshape[index0]);
				}
			}
			else {
				lastShape.insert(lastShape.begin(), anArray.getShape()[index1]);
			}
		}
		bool needBroad = false;
		for (int i = 0; i < lastShape.size(); i++) {
			if (mshape[i] != lastShape[i]) {
				needBroad = true;
				break;
			}
		}
		if (needBroad) {
			broadcast(lastShape);
			needBroad = false;
		}

		for (int i = 0; i < lastShape.size(); i++) {
			if (anArray.getShape()[i] != lastShape[i]) {
				needBroad = true;
				break;
			}
		}

		if (needBroad) {
			ShapeArray<T> cpArray(anArray.getData(), anArray.getSize());
			cpArray.reshape(anArray.getShape());
			cpArray.broadcast(lastShape);
			needBroad = false;
			for (int i = 0; i < msize; ++i) {
				mdata[i] -= cpArray.getData()[i];
			}
		}
		else {
			for (int i = 0; i < msize; ++i) {
				mdata[i] -= anArray.getData()[i];
			}
		}		
	}

	void subtract(T num) {
		for (int i = 0; i < msize; ++i) {
			mdata[i] -= num;
		}
	}

	void division(T num) {
		for (int i = 0; i < msize; ++i) {
			mdata[i] /= num;
		}
	}

	void multiply(T num) {
		for (int i = 0; i < msize; ++i) {
			mdata[i] *= num;
		}
	}


	void multiply(ShapeArray<T>& anArray) {
		int dims = (mshape.size() < anArray.getShape().size() ? mshape.size() : anArray.getShape().size());
		for (int i = 0; i < dims; ++i) {
			if ((mshape[mshape.size() - 1 - i] != 1)
				&& (anArray.getShape()[anArray.getShape().size() - 1 - i] != 1)
				&& (mshape[mshape.size() - 1 - i] != anArray.getShape()[anArray.getShape().size() - 1 - i])) {
				return;
			}
		}
		std::vector<int> lastShape;
		int index0, index1;
		int maxsize = (mshape.size() > anArray.getShape().size() ? mshape.size() : anArray.getShape().size());
		for (int i = 0; i < maxsize; ++i) {
			index1 = anArray.getShape().size() - 1 - i;
			index0 = mshape.size() - 1 - i;
			if (index0 >= 0) {
				if (index1 >= 0) {
					if (mshape[index0] != 1) {
						lastShape.insert(lastShape.begin(), mshape[index0]);
					}
					else {
						lastShape.insert(lastShape.begin(), anArray.getShape()[index1]);
					}
				}
				else {
					lastShape.insert(lastShape.begin(), mshape[index0]);
				}
			}
			else {
				lastShape.insert(lastShape.begin(), anArray.getShape()[index1]);
			}
		}
		bool needBroad = false;
		for (int i = 0; i < lastShape.size(); i++) {
			if (mshape[i] != lastShape[i]) {
				needBroad = true;
				break;
			}
		}
		if (needBroad) {
			broadcast(lastShape);
			needBroad = false;
		}

		for (int i = 0; i < lastShape.size(); i++) {
			if (anArray.getShape()[i] != lastShape[i]) {
				needBroad = true;
				break;
			}
		}

		if (needBroad) {
			ShapeArray<T> cpArray(anArray.getData(), anArray.getSize());
			cpArray.reshape(anArray.getShape());
			cpArray.broadcast(lastShape);
			needBroad = false;
			for (int i = 0; i < msize; ++i) {
				mdata[i] *= cpArray.getData()[i];
			}
		}
		else {
			for (int i = 0; i < msize; ++i) {
				mdata[i] *= anArray.getData()[i];
			}
		}
	}

        void division(ShapeArray<T>& anArray) {
		int dims = (mshape.size() < anArray.getShape().size() ? mshape.size() : anArray.getShape().size());
		for (int i = 0; i < dims; ++i) {
			if ((mshape[mshape.size() - 1 - i] != 1)
				&& (anArray.getShape()[anArray.getShape().size() - 1 - i] != 1)
				&& (mshape[mshape.size() - 1 - i] != anArray.getShape()[anArray.getShape().size() - 1 - i])) {
				return;
			}
		}
		std::vector<int> lastShape;
		int index0, index1;
		int maxsize = (mshape.size() > anArray.getShape().size() ? mshape.size() : anArray.getShape().size());
		for (int i = 0; i < maxsize; ++i) {
			index1 = anArray.getShape().size() - 1 - i;
			index0 = mshape.size() - 1 - i;
			if (index0 >= 0) {
				if (index1 >= 0) {
					if (mshape[index0] != 1) {
						lastShape.insert(lastShape.begin(), mshape[index0]);
					}
					else {
						lastShape.insert(lastShape.begin(), anArray.getShape()[index1]);
					}
				}
				else {
					lastShape.insert(lastShape.begin(), mshape[index0]);
				}
			}
			else {
				lastShape.insert(lastShape.begin(), anArray.getShape()[index1]);
			}
		}
		bool needBroad = false;
		for (int i = 0; i < lastShape.size(); i++) {
			if (mshape[i] != lastShape[i]) {
				needBroad = true;
				break;
			}
		}
		if (needBroad) {
			broadcast(lastShape);
			needBroad = false;
		}

		for (int i = 0; i < lastShape.size(); i++) {
			if (anArray.getShape()[i] != lastShape[i]) {
				needBroad = true;
				break;
			}
		}

		if (needBroad) {
			ShapeArray<T> cpArray(anArray.getData(), anArray.getSize());
			cpArray.reshape(anArray.getShape());
			cpArray.broadcast(lastShape);
			needBroad = false;
			for (int i = 0; i < msize; ++i) {
				mdata[i] /= cpArray.getData()[i];
			}
		}
		else {
			for (int i = 0; i < msize; ++i) {
				mdata[i] /= anArray.getData()[i];
			}
		}
	}

	void setSub(int axis, int start, int end, ShapeArray<T>& sub) {
		return setSub(axis, start, end, 1, sub);
	}

	void setSub(int axis, int start, int end, int step, ShapeArray<T>& sub) {
		if ((axis >= 0) && (axis >= mshape.size())) {
			return;
		}
		if ((axis < 0) && ((axis + mshape.size()) < 0)) {
			return;
		}
		int layer = (mshape.size() + axis) % mshape.size();
		if ((end - start + step - 1) / step > mshape[layer]) {
			return;
		}
		T* newdata = new T[sub.getSize()];
		sub.getLayer(-1, newdata);
		long location = 0;
		setSubArray(mdata, mshape, layer, start, end, step, newdata, location);
                delete[] newdata;
	}

	void subArray(int axis, int start, int end, ShapeArray<T>& sub) {
		return subArray(axis, start, end, 1, sub);
	}

        void subArrayLastlayer(ShapeArray<T>& mask, ShapeArray<T>& sub) {     
                if(mask.getShape().size() > 1) {
			return; 		
		}  
                T *data = new T[msize];
                long location = 0;
		int step = 1;
		if(mshape.size() > 1) {
			step = mshape[mshape.size() - 1];
		}
                for(int i=0; i< msize/step; ++i) {
                        if(mask.getData()[i] > 0) {
				memcpy((void*)(data + location*step), (void*)(mdata + i * step), step * sizeof(T));
				location ++;
			}
		}
		sub.clear(); 
                sub.setData(data, location*step);
		delete[] data;	
		std::vector<int> shape;
		shape.assign(mshape.begin(), mshape.end());
		shape[0] = location;
                sub.reshape(shape);	
	}

	void subArray(int axis, int start, int end, int step, ShapeArray<T>& sub) {
		if ((axis >= 0) && (axis >= mshape.size())) {
			return;
		}
		if ((axis < 0) && ((axis + mshape.size()) < 0)) {
			return;
		}
		int layer = (mshape.size() + axis) % mshape.size();
		if ((end - start + step - 1)/step > mshape[layer]) {
			return;
		}
		std::vector<T> outVec;
		getSubArray(mdata, mshape, layer, start, end, step, outVec);
		std::vector<int> shape;
		for (int i = 0; i < mshape.size(); ++i) {
			if (i != layer) {
				shape.push_back(mshape[i]);
			}
			else {
				shape.push_back((end - start + step - 1) / step);
			}
		}
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		if (size != outVec.size()) {
			return;
		}
		T *data = new T[size];
		for (int i = 0; i < outVec.size(); ++i) {
			data[i] = outVec[i];
		}
		sub.clear();
		sub.setData(data, size);
		sub.reshape(shape);
		delete[] data;
	}

	void getLayer(int axis, T* data) {
		if ((axis >= 0) && (axis >= mshape.size())) {
			return;
		}
		if ((axis < 0) && ((axis + mshape.size()) < 0)) {
			return;
		}
		int layer = (mshape.size() + axis) % mshape.size();
		long location = 0;
		return getLayer(mdata, mshape, layer, data, location);
	}

private:
	void printArray(T* data, long size, int layer) {
		int cols = mshape[layer++];
		int raws = size / cols;
		if (mshape.size() > layer) {
			std::cout << "[ ";
			for (int i = 0; i < cols; ++i) {
				printArray(data + i * raws, raws, layer);
			}
			std::cout << " ] ";
		}
		else {
			std::cout << "[ ";
			for (int i = 0; i < cols; ++i) {
				for (int j = 0; j < raws; ++j) {
					std::cout << " " << data[i*raws + j] << " ";
				}
			}
			std::cout << " ] " << std::endl;
		}
	}

	void argmax(T* data, const std::vector<int>& shape, int layer, std::vector<int>& maxVec) {
		if ((shape.size() - 1) < layer) {
			return;
		}
		int rows = shape[0];
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		int cols = size / rows;
		if (layer == 0) {
			for (int i = 0; i < cols; ++i) {
				T maxnum = data[i];
				int maxindex = 0;
				for (int j = 0; j < rows; ++j) {
					if (data[i + j * cols] > maxnum) {
    						maxnum = data[i + j * cols];
						maxindex = j;
					}
				}
				maxVec.push_back(maxindex);
			}
		}
		else
		{
			std::vector<int> tmpShape;
			for (int i = 1; i < shape.size(); ++i) {
				tmpShape.push_back(shape[i]);
			}
			for (int i = 0; i < rows; ++i) {
				argmax(data + i * cols, tmpShape, layer - 1, maxVec);
			}
		}
	}

	void multiply_reduce(T* data, const std::vector<int>& shape, int layer, std::vector<T>& outVec) {
		if ((shape.size() - 1) < layer) {
			return;
		}
		int rows = shape[0];
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		int cols = size / rows;
		if (layer == 0) {
			for (int i = 0; i < cols; ++i) {
				T rlt = 1;
				for (int j = 0; j < rows; ++j) {
					rlt *= data[i + j * cols];
				}
				outVec.push_back(rlt);
			}
		}
		else
		{
			std::vector<int> tmpShape;
			for (int i = 1; i < shape.size(); ++i) {
				tmpShape.push_back(shape[i]);
			}
			for (int i = 0; i < rows; ++i) {
				multiply_reduce(data + i * cols, tmpShape, layer - 1, outVec);
			}
		}			
	}
	void setSubArray(T* data, const std::vector<int>& shape, int layer, int start, int end, int step, T* newdata, long& location) {
		if ((shape.size() - 1) < layer) {
			return;
		}
		int rows = shape[0];
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		int cols = size / rows;
		if (layer == 0) {
			int loc = 0;
			for (int i = start; i < end; i += step) {
				for (int j = 0; j < cols; ++j) {
					data[i*cols + j] = newdata[location];
					location += 1;
				}
			}
		}
		else
		{
			std::vector<int> tmpShape;
			for (int i = 1; i < shape.size(); ++i) {
				tmpShape.push_back(shape[i]);
			}
			for (int i = 0; i < rows; ++i) {
				setSubArray(data + i * cols, tmpShape, layer - 1, start, end, step, newdata, location);
			}
		}
	}

	void getSubArray(T* data, const std::vector<int>& shape, int layer, int start, int end, int step, std::vector<T>& outVec) {
		if ((shape.size() - 1) < layer) {
			return;
		}
		int rows = shape[0];
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		int cols = size / rows;
		if (layer == 0) {
			for (int i = start; i < end; i+=step) {
				for (int j = 0; j < cols; ++j) {
					outVec.push_back(data[i*cols + j]);
				}
			}
		}
		else
		{
			std::vector<int> tmpShape;
			for (int i = 1; i < shape.size(); ++i) {
				tmpShape.push_back(shape[i]);
			}
			for (int i = 0; i < rows; ++i) {
				getSubArray(data + i * cols, tmpShape, layer - 1, start, end, step, outVec);
			}
		}
	}
	void getLayer(T* data, const std::vector<int>& shape, int layer, T* savedata, long& location) {
		if ((shape.size() - 1) < layer) {
			return;
		}
		int rows = shape[0];
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		int cols = size / rows;
		if (layer == 0) {
			for (int i = 0; i < cols; ++i) {
				for (int j = 0; j < rows; ++j) {
					//std::cout << data[i + j * cols];
					savedata[location] = data[i + j * cols];
					location += 1;
				}
			}
		}
		else
		{
			std::vector<int> tmpShape;
			for (int i = 1; i < shape.size(); ++i) {
				tmpShape.push_back(shape[i]);
			}
			for (int i = 0; i < rows; ++i) {
				getLayer(data + i * cols, tmpShape, layer - 1, savedata, location);
			}
		}
	}
	void printLayer(T* data, const std::vector<int>& shape, int layer) {
		if ((shape.size() - 1) < layer) {
			return;
		}
		int rows = shape[0];
		long size = shape[0];
		for (int i = 1; i < shape.size(); ++i) {
			size *= shape[i];
		}
		int cols = size / rows;
		if (layer == 0) {
			for (int i = 0; i < cols; ++i) {
				for (int j = 0; j < rows; ++j) {
					std::cout << data[i + j * cols];
				}
			}
		}
		else
		{
			std::vector<int> tmpShape;
			for (int i = 1; i < shape.size(); ++i) {
				tmpShape.push_back(shape[i]);
			}
			for (int i = 0; i < rows; ++i) {
				printLayer(data + i * cols, tmpShape, layer - 1);
			}
		}
	}
};
