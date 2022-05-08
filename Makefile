src/engine.mjs: engine/js_api.cpp engine/sovereign_chess.h engine/sovereign_chess.cpp engine/chess.cpp engine/chess.h engine/generic_bots.h
	emcc --no-entry engine/js_api.cpp engine/sovereign_chess.cpp engine/chess.cpp -o src/engine.mjs  \
		-std=c++20 \
	  -s ENVIRONMENT='web'  \
	  -s SINGLE_FILE=1  \
	  -s EXPORT_NAME='createModule'  \
	  -s USE_ES6_IMPORT_META=0  \
	  -s EXPORTED_FUNCTIONS='["_get_legal_moves", "_select_move", "_make_move"]'  \
	  -s EXPORTED_RUNTIME_METHODS='["cwrap"]'  \
		-gsource-map --source-map-base=http://127.0.0.1:8080/ \
	  -g

chess_test: engine/chess_test.cpp engine/chess.cpp engine/chess.h
	clang++ -std=c++20 -O2 -Wall engine/chess_test.cpp engine/chess.cpp -o build/chess_test

sovereign_chess_test: engine/sovereign_chess_test.cpp engine/sovereign_chess.h engine/sovereign_chess.cpp engine/chess.cpp engine/chess.h
	clang++ -std=c++20 -O2 -g -Wall engine/sovereign_chess_test.cpp engine/sovereign_chess.cpp engine/chess.cpp -o build/sovereign_chess_test