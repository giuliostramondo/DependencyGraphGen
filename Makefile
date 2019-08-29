all:
	cd ~/Projects/llvm-project/llvm/OBJ_ROOT_SLIM; make LLVMDependencyGraph -j8

test_mv:
	cd ~/Projects/benchmarks/matrix_vector_mul/;rm DependencyGraph_class_boost_generated_ddg_color_writer.dot;~/Projects/llvm-project/llvm/OBJ_ROOT/bin/opt -load ~/Projects/llvm-project/llvm/OBJ_ROOT_SLIM/./lib/LLVMDependencyGraph.so -dependencyGraph vector_matrix_mult_kernel_unrolled.bc >/dev/null;dot -Tpng DependencyGraph_class_boost_generated_ddg_color_writer.dot -o boost_generated_ddg.png; eog boost_generated_ddg.png

test_mv_100:
	cd ~/Projects/benchmarks/matrix_vector_mul_100_100/;rm DependencyGraph_class_boost_generated_ddg_color_writer.dot;~/Projects/llvm-project/llvm/OBJ_ROOT/bin/opt -load ~/Projects/llvm-project/llvm/OBJ_ROOT_SLIM/./lib/LLVMDependencyGraph.so -dependencyGraph vector_matrix_mult_kernel_unrolled.bc >/dev/null;dot -Tpng DependencyGraph_class_boost_generated_ddg_color_writer.dot -o boost_generated_ddg.png; eog boost_generated_ddg.png

clean:
	rm -rf *.ll *.dot *.csv error.log
