package main

import (
        "fmt"
        "math"
        "math/bits"
        "os"
        "github.com/damnever/bitarray"
        "bytes"
		"strconv"
       )

type cell_v struct {
   v *bitarray.BitArray
}

type cell_coord struct {
   r int
   c int
}

type sudoku struct {
    bdim int
    dim int
    peers_size int
    grid []int
    unit_list [][][][]cell_coord
    peers [][][]cell_coord
    values [][]cell_v
    sol_count uint64
}


func arrayToString(A []int) string {

    var buffer bytes.Buffer
    for i := 0; i < len(A); i++ {
        buffer.WriteString(strconv.Itoa(A[i]))
        if i != len(A)-1 {
            buffer.WriteString("")
        }
    }

    return buffer.String()
}

func digit_get(jk cell_v) int{
    count := jk.v.Count()
    n, _ := strconv.ParseUint(arrayToString(jk.v.ToArray()), 2, 64)
    if count != 1 {
        return -1;
    }
    for i := 0; i < 1; i++ {
        if n != 0 { 
            return int(uint64(64) - (uint64(1) + uint64(64*i) + uint64(bits.TrailingZeros64(n))))
        }
    }
    return -1;
}

func initr(s *sudoku) {
    var i, j, k, l, pos int
    s.unit_list = make([][][][]cell_coord, s.dim)

	for i := range s.unit_list {
    s.unit_list[i] = make([][][]cell_coord, s.dim)
    for j := range s.unit_list[i] {
        s.unit_list[i][j] = make([][]cell_coord, 3)
        for k := range s.unit_list[i][j] {
        	s.unit_list[i][j][k] = make([]cell_coord, s.dim)
        	}
    	}
	}

	s.peers = make([][][]cell_coord, s.dim)

	for i := range s.peers {
    s.peers[i] = make([][]cell_coord, s.dim)
    for j := range s.peers[i] {
        s.peers[i][j] = make([]cell_coord, s.peers_size)
    	}
	}

    s.values = make([][]cell_v, s.dim)

    for i := range s.values {
    s.values[i] = make([]cell_v, s.dim)
    }

	for i = 0; i < s.dim; i++ {
        for j = 0; j < s.dim; j++ {
        	s.values[i][j].v = bitarray.New(64)
        }
    }
    //unit list 
    for i = 0; i < s.dim; i++ {
        ibase := i / s.bdim * s.bdim;
        for j = 0; j < s.dim; j++ {
            for pos = 0; pos < s.dim; pos++ {
                s.unit_list[i][j][0][pos].r = i; //row 
                s.unit_list[i][j][0][pos].c = pos;
                s.unit_list[i][j][1][pos].r = pos; //column
                s.unit_list[i][j][1][pos].c = j;
            }
            jbase := j / s.bdim * s.bdim;
            pos:=0
            for k = 0; k < s.bdim; k++ {
                for l = 0; l < s.bdim; l++ {
                    s.unit_list[i][j][2][pos].r = ibase + k;
                    s.unit_list[i][j][2][pos].c = jbase + l;
                    pos = pos + 1
                }
            }
        }
    }
    
    //peers
    for i = 0; i < s.dim; i++ {
        for j = 0; j < s.dim; j++ {
            pos = 0;
            for k = 0; k < s.dim; k++ { //row
                if s.unit_list[i][j][0][k].c != j {
                    s.peers[i][j][pos] = s.unit_list[i][j][0][k];
                    pos = pos + 1 
                }
            }
            for k = 0; k < s.dim; k++ { 
                var sq cell_coord = s.unit_list[i][j][1][k]; //column
                if sq.r != i {
                    s.peers[i][j][pos] = sq;
                    pos = pos + 1 
                }
                sq = s.unit_list[i][j][2][k]; //box
                if sq.r != i && sq.c != j {
                    s.peers[i][j][pos] = sq;
                    pos = pos + 1
                } 
            }
        }
    }
}

func parse_grid(s *sudoku) int{
    var i, j, k int
    k = 0
    ld_vals := make([][]int, s.dim)
 
    for i := range ld_vals {
	ld_vals[i] = make([]int, s.dim)
	}

    for i = 0; i < s.dim; i++ {
        for j = 0; j < s.dim; j++ {
        	//fmt.Print(s.grid[k])
            ld_vals[i][j] = s.grid[k];
            //fmt.Print(ld_vals[i][j])
            k = k + 1;
        }
    }
    for i = 0; i < s.dim; i++ {
        for j = 0; j < s.dim; j++ {
            for k = 1; k <= s.dim; k++ {
                s.values[i][j].v.Put(k, 1);
                }
            }
    }
    for i = 0; i < s.dim; i++ { 
        for j = 0; j < s.dim; j++ {
            if ld_vals[i][j] > 0 && assign(s, i, j, ld_vals[i][j])!=1 {
                return 0;
                }
            }
        }
    return 1;
}

func create_sudoku(bdim int, grid []int) *sudoku {

    var r *sudoku = new(sudoku)
    r.bdim = bdim;
    dim := bdim * bdim;
    r.dim = dim;
    r.peers_size = 3 * dim - 2 * bdim - 1;
    r.grid = grid;
    r.sol_count = 0;
    
    initr(r);
    if parse_grid(r)!=1 {
        fmt.Println("Error parsing grid\n");
        r = nil
        return r;
    }
    
    return r;
}

func eliminate (s *sudoku, i int, j int, d int) int{
    var k, ii, cont, pos int
    el,_ := s.values[i][j].v.Get(d)
    if el != 1 {
        return 1;
    }

    s.values[i][j].v.Put(d, 0)

    count := s.values[i][j].v.Count()
    if count == 0 {
        return 0;
    } else if count == 1 {
        for k = 0; k < s.peers_size; k++ {
            if eliminate(s, s.peers[i][j][k].r, s.peers[i][j][k].c, digit_get(s.values[i][j])) != 1 { //check
                return 0;
            }
        }
    }

    for k = 0; k < 3; k++ {//row, column, box 
        cont = 0;
        pos = 0;
        //var u []cell_coord
        u := s.unit_list[i][j][k];
        for ii = 0; ii < s.dim; ii++ {
        	ch,_ := s.values[u[ii].r][u[ii].c].v.Get(d)
            if ch != 0 {
                cont++;
                pos = ii;
            }
        }
        if cont == 0 {
            return 0;
        } else if cont == 1 {
            if assign(s, u[pos].r, u[pos].c, d)!=1 {
                return 0;
            }
        }
    }
    return 1;
}

func assign (s *sudoku, i int, j int, d int) int{
    for d2 := 1; d2 <= s.dim; d2++ {
        if d2 != d {
            if eliminate(s, i, j, d2) !=1 {
               return 0;
            }
        }
    }
    return 1;
}

func display(s *sudoku) {
    fmt.Printf("%d\n", s.bdim);
    for i := 0; i < s.dim; i++ {
        for j := 0; j < s.dim; j++ {
            fmt.Printf("%d ",  digit_get(s.values[i][j])); //check
            }
            fmt.Println()
    }
}

func search (s *sudoku, status int) int{
    var i, j int

    if status!=1 {
     return status;
    }

    solved := 1;
    for i = 0; ((solved != 0) && (i < s.dim)); i++ {
        for j = 0; j < s.dim; j++ {
            if s.values[i][j].v.Count() != 1 {
                solved = 0;
                break;
            }
         } 
    }
    if solved != 0 {
        s.sol_count = s.sol_count + 1
        return 0
    }   

    //ok, there is still some work to be done

    min := math.MaxInt32;
    minI := -1;
    minJ := -1;
    ret := 0;


    values_q := make([][][]int, s.dim)

    for i := range values_q {
    values_q[i] = make([][]int, s.dim)
    for j := range values_q[i] {
        values_q[i][j] = make([]int, s.dim)
        }
    }

    for i = 0; i < s.dim; i++ {
        for j = 0; j < s.dim; j++ {
            used := s.values[i][j].v.Count()
            if used > 1 && used < min {
                min = used;
                minI = i;
                minJ = j;
            }
        }
    }
            
    for k := 1; k <= s.dim; k++ {
    	va,_ := s.values[minI][minJ].v.Get(k)
        if va != 0 {
            
            for i = 0; i < s.dim; i++ {
                for j = 0; j < s.dim; j++ {
                    for l := 0; l < s.dim; l++ {
                        values_q[i][j][l], _ = s.values[i][j].v.Get(l+1)
                        }
                    }
            }
            ass := assign(s, minI, minJ, k)
            if search(s, ass) != 0 {
                ret = 1;
                return 1
            } else {
                    for i = 0; i < s.dim; i++ {
                        for j = 0; j < s.dim; j++ {
                            for b := 0; b < s.dim; b++ {
                                if values_q[i][j][b] == 1 { 
                                s.values[i][j].v.Put(b+1, 1)
                                } else {
                                s.values[i][j].v.Put(b+1, 0)
                                }
                                }
                            }
                    }
            }
        }
    }
      
    return ret;
}

func solve(s *sudoku) int{
    return search(s, 1);
}

func main(){
    var size int
    fmt.Scanln(&size)
    buf_size := size * size * size * size;
    buf:= make ([]int, buf_size)
    for i := 0; i < buf_size; i++ {
        _, err:= fmt.Scanf("%d", &buf[i])
        if err!=nil {
            fmt.Println("error while reading file")
            os.Exit(1);
        }

    }
    s := create_sudoku(size, buf);
    if s!=nil {
        display(s)
        solve(s);
        if s.sol_count > 0 {
                    fmt.Print(s.sol_count)
                    fmt.Print("\n")
            
            } else {
                fmt.Println("Could not solve puzzle.\n");
            }
        } else {
            fmt.Println("Could not solve puzzle.\n");
        }
}

