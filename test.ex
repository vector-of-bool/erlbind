defmodule Erlbind.Test do
  @on_load :load_nifs

  def load_nifs do
    :ok = :erlang.load_nif('./build/my-nifs', 0)
    12 = test_fn()
    :ok
  end

  def test_fn do
    raise "Badness"
  end
end