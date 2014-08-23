#include <memory>
#include <vector>
#include <fstream>

namespace cm
{
	struct Simulation;
	struct Cloth;
	struct Mesh;
	struct Timer;
	struct Velocity;

	class ClothHandler
	{
	public:
		typedef double * const DoubleDataBuffer;
		typedef int * const IntDataBuffer;

		ClothHandler();

		void init_avatars_to_handler(
			DoubleDataBuffer position, 
			DoubleDataBuffer texcoords, 
			IntDataBuffer indices,
			size_t faceNum
			);
		void add_clothes_to_handler(
			DoubleDataBuffer position, 
			DoubleDataBuffer texcoords, 
			IntDataBuffer indices,
			size_t faceNum
			);
		void update_avatars_to_handler(DoubleDataBuffer position);

		// Temporary used to import obj cloth file
		void add_clothes_to_handler(const char * filename);
		void fill_buffer();
		std::vector<float> get_position() { return position_buffer_; }
		std::vector<float> get_normal() { return normal_buffer_; }

		void begin_simulate();
		void sim_next_step();
		bool load_cmfile_to_replay(const char * fileName);

		size_t face_count();
		size_t cloth_num() { return clothes_.size(); }

	private:
		void init_simulation();
		void init_cmfile(const char * fileName);
		void write_frame_to_cmfile(int frame);
		void save_cmfile();

		void apply_velocity(Mesh &mesh, const Velocity &vel);

		std::tr1::shared_ptr<Simulation> sim_;
		int frame_;
		std::tr1::shared_ptr<Timer> fps_;
		std::vector<std::tr1::shared_ptr<Cloth> > clothes_;
		std::vector<std::tr1::shared_ptr<Cloth> > clothes_frame_;
		std::vector<float> position_buffer_;
		std::vector<float> normal_buffer_;
		std::ofstream clothMotionFile_;
	};
};